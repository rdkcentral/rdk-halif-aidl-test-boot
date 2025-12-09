#!/usr/bin/env python3
#** *****************************************************************************
# *
# * If not stated otherwise in this file or this component's LICENSE file the
# * following copyright and licenses apply:
# *
# * Copyright 2024 RDK Management
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *
# http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *
#* ******************************************************************************

import os
import sys
import tarfile
import yaml
import time

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))

from raft.framework.plugins.ut_raft.configRead import ConfigRead
from raft.framework.plugins.ut_raft import utHelperClass
from raft.framework.core.logModule import logModule
from raft.framework.plugins.ut_raft.utSuiteNavigator import UTSuiteNavigatorClass
from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils
from raft.framework.plugins.ut_raft.utUserResponse import utUserResponse
from raft.framework.core.utPlaneController import utPlaneController

BOOT_TEST_MENU_CONFIGURATION_FILE = os.path.join(dir_path, "vts_boot_testMenuConfiguration.yaml")
BOOT_TESTCASES_CONFIGURATION_FILE = os.path.join(os.path.join(dir_path, "../configs"), "vts_boot_testcase_configuration.yaml")
BOOT_L3_TEST_CONFIG_FILE = os.path.join(os.path.join(dir_path, "../../test_configuration"), "vts_l3_boot.yaml")

class bootHelperClass(utHelperClass):

    def __init__(self, testName:str, qcId:str, log:logModule=None ):
        """
        Initializes the test class
        """
        self.moduleName = "Boot"
        self.profile_moduleName = "IBoot"
        super().__init__(testName, qcId, log)

        self.log = log
        if log is None:
            self.log = logModule(self.__class__.__name__)
            self.log.setLevel(self.log.INFO)

        # Parse device configuration
        self.deviceTestSetup = self.cpe.get("test")
        self.targetWorkspace = os.path.join(self.cpe.get("target_directory"), self.moduleName)
        self.moduleHFPFile = os.path.join(dir_path, self.deviceTestSetup.get("profile"))
        profileOnTarget = os.path.join(self.targetWorkspace, "profiles")
        profileOnTarget = os.path.join(profileOnTarget, os.path.basename(self.moduleHFPFile))
        profileOnTarget = profileOnTarget.replace("\\", "/")

        platformConfigOnTarget = os.path.join(self.targetWorkspace, "test_configuration")
        platformConfigOnTarget = platformConfigOnTarget.replace("\\", "/")

        self.l3TestConfigFolder = os.path.join(self.targetWorkspace, "test_configuration")
        self.l3TestConfigFolder = self.l3TestConfigFolder.replace("\\", "/")

        self.waitTime = self.cpe.get("wait_time")
        self.device_type = self.cpe.get("device_type")
        self.platform = self.cpe.get("platform")

        # Load test menu and test case configuration files
        self.l3TestConfig = ConfigRead(BOOT_L3_TEST_CONFIG_FILE, self.moduleName)
        self.testMenuConfiguration = ConfigRead(BOOT_TEST_MENU_CONFIGURATION_FILE, self.moduleName)
        self.testCaseConfiguration = ConfigRead(BOOT_TESTCASES_CONFIGURATION_FILE, self.moduleName)
        self.testMenuConfiguration.test.execute = self.testMenuConfiguration.test.execute + f" -p {profileOnTarget}"

        # Establish HAL session
        self.hal_session = self.dut.getConsoleSession("ssh_hal_test")
        self.hal_session_1 = self.dut.getConsoleSession("ssh_hal_test_1")

        # Initialize UT menu navigator and utilities
        self.utMenu = UTSuiteNavigatorClass(self.testMenuConfiguration, None, self.hal_session)
        self.utilities = utBaseUtils()

        # Create user response Class
        self.testUserResponse = utUserResponse()

        # Copy required artifacts to target device
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        for artifact in self.testMenuConfiguration.test.artifacts:
            filesPath = os.path.join(dir_path, artifact)
            self.utilities.sftpCopy(self.hal_session, filesPath, self.targetWorkspace)

            if tarfile.is_tarfile(filesPath):
                self.utilities.change_directory(self.hal_session, self.targetWorkspace)
                target_tarfile = os.path.join(self.targetWorkspace, os.path.basename(artifact))
                target_tarfile = target_tarfile.replace("\\", "/")
                self.utilities.untar(self.hal_session, target_tarfile, self.targetWorkspace)

        self.utilities.change_directory(self.hal_session_1, self.targetWorkspace)
        self.hal_session_1.write(f"./{self.cpe.get('boot_process_name')} &")
        time.sleep(2)

    def testEndFunction(self, powerOff=True):
        """
        Ends the test case.
        Args:
            powerOff (bool): Whether to power off the device
        """
        super().testEndFunction(powerOff)

    def waitForBoot(self):
        return super().waitForBoot()
    
    def triggerBoot(self, trigger: str) -> bool:
        """
        This function sends a specified boot reason for booting up the device via ut-controller.

        Args:
          trigger (str): boot reason to be sent for booting up the device

        Returns:
          bool: True if the message was sent successfully, False otherwise.
        """
        yaml_content = ()
        if trigger == "WATCHDOG":
            yaml_content = (
                "Boot:\n"
                "  command: \"WATCHDOG\"\n"
            )
        elif trigger == "SOFTWARE_RESET":
            yaml_content = (
                "Boot:\n"
                "  command: \"SOFTWARE_RESET\"\n"
            )
        elif trigger == "THERMAL_RESET":
            yaml_content = (
                "Boot:\n"
                "  command: \"THERMAL_RESET\"\n"
            )
        elif trigger == "COLD_BOOT":
            yaml_content = (
                "Boot:\n"
                "  command: \"COLD_BOOT\"\n"
            )
        elif trigger == "STR_AUTH_FAILURE":
            yaml_content = (
                "Boot:\n"
                "  command: \"STR_AUTH_FAILURE\"\n"
            )
        else:
            print("Unknown trigger\n")

        # Send the command to ut-controller
        self.controller.sendMessage(yaml_content)

        return True

    def rebootWithBootReason(self, expectedBootReason: str) -> bool:
        """
        This function verifies the boot reason after reboot.

        Args:
          expectedBootReason (str): The expected boot reason to verify against.

        Returns:
          bool: True if the boot reason matches the expected value, False otherwise.
        """
        self.hal_session = self.dut.getConsoleSession("default")
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        # Trigger reboot with specified boot reason based on platform
        if self.platform.lower() == "vDevice".lower():
            self.deviceTestSetup = self.cpe.get("test")
            self.controller = utPlaneController(self.session, port=self.deviceTestSetup.get("controller_port"), log=self.log)
            self.port = self.deviceTestSetup.get("controller_port")
            self.triggerBoot(expectedBootReason)
        else:
            input(f"Please bootup {self.platform} with the specified boot reason: {expectedBootReason} and press Enter to continue...")
        
        sleepTime = self.cpe.get("wait_time")
        self.log.info(f"Waiting for {sleepTime} seconds to allow device to reboot...")
        time.sleep(sleepTime)  # Wait for the configured time to allow device to reboot
        self.hal_session.close()
        result = self.waitForBoot()
        if not result:
            self.log.error(f"Device did not boot up successfully after {expectedBootReason} reboot.")
            return False
        
    def runTestCaseWithBootReason(self, expectedBootReason: str) -> bool:
        """
        Runs the test case.

        Returns:
            bool: True if the test case passed, False otherwise.
        """
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        self.testcase = "checkBootReason"
        self.profileConfiguration = ConfigRead(self.moduleHFPFile, self.profile_moduleName)

        self.utMenu.start()

        output =self.utMenu.select("VTS_L3_BOOT", self.testcase, None, self.waitTime, False)
        results = self.utMenu.collect_results(output, True)
        
        # Validate Boot Reason WATCHDOG in results
        bootReasonActual = output.strip()
        if expectedBootReason in bootReasonActual:
            self.log.info(f"Boot Reason '{expectedBootReason}' verified successfully after reboot.")

        self.log.stepResult(results, f'Test Suit: {"VTS_L3_BOOT"} Test Case: {self.testcase}')