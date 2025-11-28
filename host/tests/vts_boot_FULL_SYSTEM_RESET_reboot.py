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
import time

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))

from raft.framework.plugins.ut_raft.configRead import ConfigRead
from raft.framework.core.logModule import logModule
from raft.framework.plugins.ut_raft import utHelperClass
from raft.framework.plugins.ut_raft.utSuiteNavigator import UTSuiteNavigatorClass
from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils
from vts_boot_helper import bootHelperClass

BOOT_L3_TEST_SUITE_NAME = "VTS_L3_BOOT"
NON_PERSISTENT_FILE_PATH = "/tmp/"
L3_TEST_CONFIG_FILE_NAME = "vts_l3_boot.txt"

class  VTS_BOOT_L3_Reboot(bootHelperClass):
    """
    Runs L3 Set States test
    """
    def __init__(self, log:logModule=None):
        """
        Initializes the test class
        """
        self.testName = "BOOT_L3_Reboot_FULL_SYSTEM_RESET"
        qcId = "1"
        
        super().__init__(self.testName, qcId, log)

    def testFunction(self):
        """
        The main test function to run the L3 Boot reboot with reset type = FULL_SYSTEM_RESET.

        Returns:
            bool: Final result of the test.
        """
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        results = False
        result = False
        self.testcase = "reboot_system"
        
        self.profileConfiguration = ConfigRead(self.moduleHFPFile, self.profile_moduleName)

        self.utilities.sftpCopy(self.hal_session, self.l3TestConfigFile, self.l3TestConfigFolder)
        # Create a non-persistent file to test FULL_SYSTEM_RESET reboot
        self.hal_session.write(f"touch {NON_PERSISTENT_FILE_PATH}{L3_TEST_CONFIG_FILE_NAME}")

        self.utMenu.start()
        promptWithAnswers = [
                {    
                    "query_type": "direct",
                    "query": "Enter reset types from below options:",
                    "input": str(0) # FULL_SYSTEM_RESET
                }    
        ]

        self.utMenu.select(BOOT_L3_TEST_SUITE_NAME, self.testcase, promptWithAnswers, self.waitTime, False)
        sleepTime = self.cpe.get("wait_time")
        self.log.info(f"Waiting for {sleepTime} seconds to allow device to reboot...")
        time.sleep(sleepTime)  # Wait for the configured time to allow device to reboot
        self.hal_session.close()
        result = self.waitForBoot()
        if not result:
            self.log.error("Device did not boot up successfully after FULL_SYSTEM_RESET reboot.")
            return False
        # Check if the non-persistent file is deleted after reboot
        fileCheckCommand = f"ls {NON_PERSISTENT_FILE_PATH}{L3_TEST_CONFIG_FILE_NAME}"
        self.hal_session = self.dut.getConsoleSession("default")
        self.hal_session.write(fileCheckCommand)
        output = self.hal_session.read_until(self.cpe.get("prompt"), timeout=10)
        if f"No such file or directory" in output:
            self.log.info("Non-persistent file is deleted after FULL_SYSTEM_RESET reboot. Test Passed.")
            results = True
        else:
            self.log.error("Non-persistent file still exists after FULL_SYSTEM_RESET reboot. Test Failed.")
            results = False
        return results

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summery"
    summaryLog  = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_BOOT_L3_Reboot(summaryLog )
    test.run(False)
