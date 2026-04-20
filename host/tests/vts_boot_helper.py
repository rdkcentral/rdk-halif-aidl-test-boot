#!/usr/bin/env python3
#* ******************************************************************************
#*  If not stated otherwise in this file or this component's LICENSE
#*  file the following copyright and licenses apply:
#*
#*  Copyright 2026 RDK Management
#*
#*  Licensed under the Apache License, Version 2.0 (the License);
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*  http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an AS IS BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#*
#* ******************************************************************************

import os
import sys
import tarfile
import time
import re
import yaml
from typing import Literal

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))
sys.path.append(os.path.join(dir_path, "../dutReboot"))
sys.path.append(os.path.join(dir_path, "../dutReboot"))

from raft.framework.plugins.ut_raft.configRead import ConfigRead
from raft.framework.plugins.ut_raft import utHelperClass
from raft.framework.core.logModule import logModule
from raft.framework.plugins.ut_raft.utSuiteNavigator import UTSuiteNavigatorClass
from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils
from raft.framework.plugins.ut_raft.utUserResponse import utUserResponse
from dutRebootController import DutRebootController
from dutRebootController import DutRebootController

BOOT_TEST_MENU_CONFIGURATION_FILE = os.path.join(dir_path, "vts_boot_testMenuConfiguration.yaml")
BOOT_TESTCASES_CONFIGURATION_FILE = os.path.join(os.path.join(dir_path, "../configs"), "vts_boot_testcase_configuration.yaml")
BOOT_L3_TEST_CONFIG_FILE = os.path.join(os.path.join(dir_path, "../../test_configuration"), "vts_l3_boot.yaml")

class bootHelperClass(utHelperClass):

    def __init__(self, testName:str, qcId:str, log:logModule=None):
        """
        Initializes the test class
        """
        self.moduleName = "Boot"
        self.profile_moduleName = "Boot"
        super().__init__(testName, qcId, log)

        self.log = log
        if log is None:
            self.log = logModule(self.__class__.__name__)
            self.log.setLevel(self.log.INFO)

        # Parse device configuration
        self.deviceTestSetup = self.cpe.get("test")
        self.targetWorkspace = os.path.join(self.cpe.get("target_directory"), self.moduleName)
        self.moduleHFPFile = os.path.join(dir_path, self.deviceTestSetup.get("profile"))
        self.nonvolatileDirectoryOnDevice = self.cpe.get("boot_non_volatile_directory")
        self.volatileDirectoryOnDevice = self.cpe.get("boot_volatile_directory")
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

        # Initialize DUT Reboot Controller
        control_port = self.dut.getField("control_port") if self.platform == "vDevice" else None
        self.dutRebootController = DutRebootController(
            log=self.log,
            platform=self.platform,
            session=self.session if self.platform == "vDevice" else None,
            control_port=control_port if control_port else 8080
        )

        # Initialize DUT Reboot Controller
        control_port = self.dut.getField("control_port") if self.platform == "vDevice" else None
        self.dutRebootController = DutRebootController(
            log=self.log,
            platform=self.platform,
            session=self.session if self.platform == "vDevice" else None,
            control_port=control_port if control_port else 8080
        )

    def testEndFunction(self, powerOff=True):
        """
        Ends the test case.
        Args:
            powerOff (bool): Whether to power off the device
        """
        super().testEndFunction(powerOff)

    def pingTest(self, deviceName="dut", logPingTime=False):
        return super().pingTest(deviceName, logPingTime)
    
    def externallyTriggeredReboot(self, expectedBootReason: str) -> bool:
        """
        Simulates an externally triggered reboot (e.g. watchdog, thermal reset) by sending
        a boot reason command via the Control Plane. Verifies the device comes back up successfully.

        Args:
          expectedBootReason (str): The boot reason to trigger the reboot with.

        Returns:
          bool: True if the device rebooted and came back up successfully, False otherwise.
        """
        self.hal_session = self.dut.getConsoleSession("default")
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        # Trigger reboot with specified boot reason using the controller
        if not self.dutRebootController.triggerReboot(expectedBootReason):
            self.log.error(f"Failed to trigger reboot with boot reason: {expectedBootReason}")
            return False
        # Trigger reboot with specified boot reason using the controller
        if not self.dutRebootController.triggerReboot(expectedBootReason):
            self.log.error(f"Failed to trigger reboot with boot reason: {expectedBootReason}")
            return False
        
        sleepTime = self.cpe.get("wait_time")
        self.log.info(f"Waiting for {sleepTime} seconds to allow device to reboot...")
        time.sleep(sleepTime)  # Wait for the configured time to allow device to reboot
        self.hal_session.close()
        result = self.pingTest(logPingTime=True)
        if not result:
            self.log.error(f"Device did not boot up successfully after {expectedBootReason} reboot.")
            return False
        return result

    def performAndVerifyReboot(self, suite_name: str, reset_index: int, reset_type: str) -> bool:
        """
        Performs reboot using UT menu and verifies device boots successfully.
        
        Args:
            suite_name: The UT test suite name
            reset_index: The index of reset type in the UT menu
            reset_type: The reset type name for logging
        
        Returns:
            bool: True if reboot and device boot verification succeeded, False otherwise
        """
        testcase = "rebootSystem"
        self.utMenu.start()
        promptWithAnswers = [
            {    
                "query_type": "direct",
                "query": "Enter reset types from below options:",
                "input": str(reset_index)
            }    
        ]

        menuOutput = self.utMenu.select(suite_name, testcase, promptWithAnswers, self.waitTime, False)
        self.log.info(menuOutput)
        sleepTime = self.cpe.get("wait_time")
        self.log.info(f"Waiting for {sleepTime} seconds to allow device to reboot...")
        time.sleep(sleepTime)
        self.hal_session.close()
        
        # Verify device booted successfully
        result = self.pingTest(logPingTime=True)
        if not result:
            self.log.error(f"Device did not boot up successfully after {reset_type} reboot.")
            return False
        
        return True

    def performAndVerifyReboot(self, suite_name: str, reset_index: int, reset_type: str) -> bool:
        """
        Performs reboot using UT menu and verifies device boots successfully.
        
        Args:
            suite_name: The UT test suite name
            reset_index: The index of reset type in the UT menu
            reset_type: The reset type name for logging
        
        Returns:
            bool: True if reboot and device boot verification succeeded, False otherwise
        """
        testcase = "rebootSystem"
        self.utMenu.start()
        promptWithAnswers = [
            {    
                "query_type": "direct",
                "query": "Enter reset types from below options:",
                "input": str(reset_index)
            }    
        ]

        menuOutput = self.utMenu.select(suite_name, testcase, promptWithAnswers, self.waitTime, False)
        self.log.info(menuOutput)
        sleepTime = self.cpe.get("wait_time")
        self.log.info(f"Waiting for {sleepTime} seconds to allow device to reboot...")
        time.sleep(sleepTime)
        self.hal_session.close()
        
        # Verify device booted successfully
        result = self.pingTest(logPingTime=True)
        if not result:
            self.log.error(f"Device did not boot up successfully after {reset_type} reboot.")
            return False
        
        return True

    def verifyBootReason(self, reset_type: str, expectedBootReason: str, test_id: str = None) -> bool:
        """
        Checks boot reason using UT menu and verifies it matches the expected boot reason.
        
        Args:
            reset_type: The type of reset that was performed (for logging)
            expectedBootReason: The expected boot reason to verify against
            test_id: Optional test ID for more specific logging
        
        Checks boot reason using UT menu and verifies it matches the expected boot reason.
        
        Args:
            reset_type: The type of reset that was performed (for logging)
            expectedBootReason: The expected boot reason to verify against
            test_id: Optional test ID for more specific logging
        
        Returns:
            bool: True if boot reason matches expected, False otherwise
        """
        # Write expectedBootReason into the L3 config YAML under ValidateBootReason,
        # then copy the updated file to the target so the UT binary can validate against it.
        with open(BOOT_L3_TEST_CONFIG_FILE, "r") as f:
            l3Config = yaml.safe_load(f)
        l3Config[self.moduleName]["L3"]["ValidateBootReason"] = expectedBootReason
        with open(BOOT_L3_TEST_CONFIG_FILE, "w") as f:
            yaml.dump(l3Config, f, default_flow_style=False)
        self.utilities.sftpCopy(self.hal_session, BOOT_L3_TEST_CONFIG_FILE, self.l3TestConfigFolder)

        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        self.testcase = "checkBootReason"
        self.hal_session = self.dut.getConsoleSession("default")
        self.hal_session = self.dut.getConsoleSession("default")
        self.utMenu.start()

        result = self.utMenu.select("VTS_L3_BOOT", self.testcase, None, self.waitTime, False)
        
        # Try multiple regex patterns to extract boot reason
        patterns = [
            r"Result getBootReason\(OUT:bootReason\), status:\[.*?bootReason:(?P<bootReason>\w+)\]",
            r"bootReason:(?P<bootReason>[A-Z_]+)"
        ]
        
        bootReason = None
        for pattern in patterns:
            match = re.search(pattern, result)
            if match:
                bootReason = match.group("bootReason")
                break
        
        if not bootReason:
            context = f"test {test_id}" if test_id else f"{reset_type} reboot"
            self.log.error(f"Failed to parse boot reason from UT menu output after {context}.")
            return False

        results = self.utMenu.collect_results(result, True)
        if not results:
            context = f"test {test_id}" if test_id else f"{reset_type} reboot"
            self.log.error(f"Failed to verify boot reason after {context}.")
            return False
        
        # Verify boot reason matches expected
        test_case = test_id if test_id else f"{reset_type}_{expectedBootReason}"
        if bootReason == expectedBootReason:
            self.log.stepResult(True, f"Test Suit: {self.testName} Test Case: {test_case}")
            return True
        else:
            self.log.stepResult(False, f"Test Suit: {self.testName} Test Case: {test_case}")
            return False

    def overrideBootReason(self, suite_name: str, boot_reason_index: int, boot_reason: str) -> bool:
        """
        Sets boot reason override using UT menu.
        
        Args:
            suite_name: The UT test suite name
            boot_reason_index: The index of boot reason in the UT menu
            boot_reason: The boot reason name for logging
        
        Returns:
            bool: True if override was set successfully, False otherwise
        """
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        testcase = "overrideBootReason"
        self.utMenu.start()
        promptWithAnswers = [
            {    
                "query_type": "direct",
                "query": "Enter boot reasons from below options:",
                "input": str(boot_reason_index)
            }
        ]
        result = self.utMenu.select(suite_name, testcase, promptWithAnswers, self.waitTime, False)
        self.log.info(f"Set boot reason override to {boot_reason} with reason string: {boot_reason}")
        return True

    def removeFilesOnDevice(self, files: list):
        """
        Removes specified files from the device.

        Args:
            files (list): List of file paths to be removed from the device.
        """
        for file in files:
            rmOutput = self.hal_session.write(f"rm {file}")
            if rmOutput is True:
                self.log.info(f"{file} removed successfully")
            else:
                self.log.error(f"Unable to remove {file}")

    def parseFileCheckLog(self, log: str) -> bool:
        """
        Parse shell log output and determine file existence.
        Returns:
            bool:
                True  -> file exists
                False -> file does NOT exist
        Assumes:
            TRUE  means file exists
            FALSE means file does not exist
        """
        if not log or not isinstance(log, str):
            raise ValueError("Log must be a non-empty string.")

        normalized = log.replace("\r\n", "\n").replace("\r", "\n")
        matches = re.findall(r"^(TRUE|FALSE)$", normalized, re.MULTILINE)

        if not matches:
            self.log.error("No TRUE/FALSE result found in log.")
            return False

        last_result: Literal["TRUE", "FALSE"] = matches[-1]
        return last_result == "TRUE"