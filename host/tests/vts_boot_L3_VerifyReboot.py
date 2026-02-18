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
from re import match
import re
import sys
import time
import subprocess

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))

from vts_boot_helper import bootHelperClass
from raft.framework.core.logModule import logModule

BOOT_L3_TEST_SUITE_NAME = "VTS_L3_BOOT"
L3_TEST_CONFIG_FILE_NAME = "vts_l3_boot.txt"

class  vts_boot_L3_VerifyReboot(bootHelperClass):
    """
    Runs L3 Set States test
    """
    def __init__(self, log:logModule=None):
        """
        Initializes the test class
        """
        self.testName = "BOOT_L3_Reboot"
        qcId = "3"
        
        super().__init__(self.testName, qcId, log)


    def fileExistsOnDevice(self, filePath, prompt=None, session=None):
        result = self.catFile(filePath, prompt=prompt, session=session)
        # If the file does not exist, typical shell returns "No such file or directory"
        if not result or "No such file or directory" in result:
           return False
        return True

    def testFunction(self):
        """
        The main test function to run the L3 Boot reboot with reset type = MAINTENANCE_REBOOT and SOFTWARE_REBOOT.

        Returns:
            bool: Final result of the test.
        """
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        results = True

        for index, reset_type in enumerate(self.l3TestConfig.L3.ResetTypes):
            self.log.stepStart(f"Testing {reset_type}...")
            self.testcase = "rebootSystem"
            
            # Define test parameters based on reset type
            if reset_type in ["MAINTENANCE_REBOOT", "SOFTWARE_REBOOT"]:
                file_directory = self.volatileDirectoryOnDevice
                file_should_persist = True
                expected_boot_reason = "MAINTENANCE_REBOOT" if reset_type == "MAINTENANCE_REBOOT" else "WARM_RESET"
            elif reset_type == "FULL_SYSTEM_RESET":
                file_directory = self.nonvolatileDirectoryOnDevice
                file_should_persist = False
                expected_boot_reason = "WARM_RESET"
            else:
                file_directory = None
                file_should_persist = None
                expected_boot_reason = "WARM_RESET"
            
            # Create test file if needed
            if file_directory:
                touchOutput = self.hal_session.write(f"touch {file_directory}{L3_TEST_CONFIG_FILE_NAME}")
                if touchOutput is True:
                    self.log.step(f"File created before {reset_type} reboot...")
                else:
                    self.log.error(f"Failed to create file before {reset_type} reboot.")
                    results = False
                    continue
            
            # Perform reboot using UT menu
            self.utMenu.start()
            promptWithAnswers = [
                {    
                    "query_type": "direct",
                    "query": "Enter reset types from below options:",
                    "input": str(index)  # As the UT menu expects an index input for reset type selection
                }    
            ]

            menuOutput = self.utMenu.select(BOOT_L3_TEST_SUITE_NAME, self.testcase, promptWithAnswers, self.waitTime, False)
            self.log.info(menuOutput)
            sleepTime = self.cpe.get("wait_time")
            self.log.info(f"Waiting for {sleepTime} seconds to allow device to reboot...")
            time.sleep(sleepTime)
            self.hal_session.close()
            
            # Verify device booted successfully
            result = self.pingTest(logPingTime=True)
            if not result:
                self.log.error(f"Device did not boot up successfully after {reset_type} reboot.")
                results = False
                continue
            
            super().__init__(self.testName, "3", self.log) #Reinitialize after reboot
            
            # Check boot reason using UT menu
            self.testcase = "checkBootReason"
            self.hal_session = self.dut.getConsoleSession("default")
            self.utMenu.start()
            result = self.utMenu.select(BOOT_L3_TEST_SUITE_NAME, self.testcase, None, self.waitTime, False)
            pattern = r"Result getBootReason\(OUT:bootReason\), status:\[.*?bootReason:(?P<bootReason>\w+)\]"
            match = re.search(pattern, result)
            if match:
                bootReason = match.group("bootReason")
            else:
                self.log.error(f"Failed to parse boot reason from UT menu output after {reset_type} reboot.")
                results = False
                continue

            results = self.utMenu.collect_results(result, True)
            if not results: 
                self.log.error(f"Failed to verify boot reason after {reset_type} reboot.")
                results = False
                continue
            
            # Verify boot reason matches expected
            if bootReason == expected_boot_reason:
                results = results and True
                self.log.stepResult(results, f"Boot reason '{bootReason}' matches expected '{expected_boot_reason}'.")
            else:
                results = False
                self.log.stepResult(results, f"Boot reason '{bootReason}' does not match expected '{expected_boot_reason}'.")
                continue
            
            # Verify file persistence if applicable
            if file_directory:

                fileCheckCommand = f"[ ! -e {file_directory}{L3_TEST_CONFIG_FILE_NAME} ] && echo 'FALSE' || echo 'TRUE'"
                self.hal_session = self.dut.getConsoleSession("default")
                self.hal_session.write(fileCheckCommand)
                time.sleep(2)  # Wait for command to execute and output to be ready
                output = self.hal_session.read_until(self.cpe.get("prompt"), timeout=10)
                self.log.info(f"File check command output: {output}")
                
                file_exists = self.parseFileCheckLog(output)
                
                if file_should_persist and file_exists:
                    results = results and True
                    self.log.stepResult(results, f"File still exists after {reset_type} reboot as expected. Test Passed.")
                elif not file_should_persist and not file_exists:
                    results = results and True
                    self.log.stepResult(results, f"File is deleted after {reset_type} reboot as expected. Test Passed.")
                elif file_should_persist and not file_exists:
                    results = False
                    self.log.stepResult(results, f"File was deleted after {reset_type} reboot unexpectedly. Test Failed.")
                    continue
                else:  # not file_should_persist and file_exists
                    results = False
                    self.log.stepResult(results, f"File still exists after {reset_type} reboot unexpectedly. Test Failed.")
                    continue
                
                # Clean up the file if it still exists
                if file_exists:
                    self.hal_session.write(f"rm {file_directory}{L3_TEST_CONFIG_FILE_NAME}")
            else:
                # For other reset types, just log success
                self.log.stepResult(results, f"Device rebooted successfully with {reset_type}. Test Passed.")
        
        return results

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = vts_boot_L3_VerifyReboot(summaryLog )
    test.run(False)
