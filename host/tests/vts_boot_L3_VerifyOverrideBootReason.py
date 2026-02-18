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
import re
import sys
import time

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))

from vts_boot_helper import bootHelperClass
from raft.framework.core.logModule import logModule

BOOT_L3_TEST_SUITE_NAME = "VTS_L3_BOOT"

class  vts_boot_L3_VerifyOverrideBootReason(bootHelperClass):
    """
    Runs L3 test with override boot reason and verifies the boot reason is set correctly after reboot.
    Tests two matrices:
    - Matrix 2A: Override with SOFTWARE_REBOOT (Should preserve override)
    - Matrix 2B: Override with Other Reset Types (Should ignore override)
    """
    def __init__(self, log:logModule=None):
        """
        Initializes the test class
        """
        self.testName = "BOOT_L3_OverrideBootReason"
        qcId = "4"
        
        super().__init__(self.testName, qcId, log)

        # Load test matrices from YAML file using ConfigRead class
        try:
            matrix_2a_config = self.l3TestConfig.L3.OverrideBootReasonMatrix2A
            matrix_2b_config = self.l3TestConfig.L3.OverrideBootReasonMatrix2B
            
            # Convert ConfigRead objects to list of dictionaries
            self.matrix_2a_tests = []
            self.matrix_2b_tests = []
            
            # Extract tests from Matrix 2A (dictionary structure)
            if hasattr(matrix_2a_config, '__dict__'):
                for test_id, test_data in matrix_2a_config.__dict__.items():
                    if test_id.startswith('OVR-'):  # Filter out internal attributes
                        if hasattr(test_data, '__dict__'):
                            test_dict = {
                                'testId': test_data.__dict__.get('testId', test_id),
                                'bootReason': test_data.__dict__.get('bootReason', ''),
                                'bootReasonIndexOverride': test_data.__dict__.get('bootReasonIndexOverride', None),
                                'reasonString': test_data.__dict__.get('reasonString', ''),
                                'resetType': test_data.__dict__.get('resetType', ''),
                                'expectedBootReason': test_data.__dict__.get('expectedBootReason', '')
                            }
                            self.matrix_2a_tests.append(test_dict)
            
            # Extract tests from Matrix 2B (dictionary structure)
            if hasattr(matrix_2b_config, '__dict__'):
                for test_id, test_data in matrix_2b_config.__dict__.items():
                    if test_id.startswith('OVR-'):  # Filter out internal attributes
                        if hasattr(test_data, '__dict__'):
                            test_dict = {
                                'testId': test_data.__dict__.get('testId', test_id),
                                'bootReason': test_data.__dict__.get('bootReason', ''),
                                'bootReasonIndexOverride': test_data.__dict__.get('bootReasonIndexOverride', None),
                                'reasonString': test_data.__dict__.get('reasonString', ''),
                                'resetType': test_data.__dict__.get('resetType', ''),
                                'expectedBootReason': test_data.__dict__.get('expectedBootReason', '')
                            }
                            self.matrix_2b_tests.append(test_dict)
            
            self.log.info(f"Matrix 2A loaded: {len(self.matrix_2a_tests)} tests")
            self.log.info(f"Matrix 2B loaded: {len(self.matrix_2b_tests)} tests")
            
        except Exception as e:
            self.log.error(f"Error loading test matrices: {e}")
            self.matrix_2a_tests = []
            self.matrix_2b_tests = []
        
        # Ensure matrices are not None
        if self.matrix_2a_tests is None:
            self.matrix_2a_tests = []
        if self.matrix_2b_tests is None:
            self.matrix_2b_tests = []
        
        # Debug first item
        if self.matrix_2a_tests:
            self.log.info(f"Matrix 2A first item: {self.matrix_2a_tests[0]}")
        
        self.log.info(f"Final Matrix 2A tests count: {len(self.matrix_2a_tests)}")
        self.log.info(f"Final Matrix 2B tests count: {len(self.matrix_2b_tests)}")

    def run_override_test(self, test_config):
        """
        Runs a single override boot reason test
        
        Args:
            test_config (dict): Test configuration containing bootReason, resetType, expectedBootReason, etc.
            
        Returns:
            bool: True if test passed, False otherwise
        """
        # Validate test_config has all required keys
        required_keys = ["testId", "bootReason", "reasonString", "resetType", "expectedBootReason"]
        for key in required_keys:
            if key not in test_config:
                self.log.error(f"Test configuration missing required key: {key}")
                return False
        
        testId = test_config["testId"]
        bootReason = test_config["bootReason"]
        reasonString = test_config["reasonString"]
        resetType = test_config["resetType"]
        expectedBootReason = test_config["expectedBootReason"]
        
        self.log.stepStart(f"Test {testId}: Setting boot reason to {bootReason}, then rebooting with {resetType}")
        
        # Get boot reason index from config
        # Check if there's an override index specified in the test config
        if 'bootReasonIndexOverride' in test_config and test_config['bootReasonIndexOverride'] is not None:
            bootReasonIndex = test_config['bootReasonIndexOverride']
            self.log.info(f"Using override index {bootReasonIndex} for boot reason {bootReason}")
        else:
            # Use default index from BootReason list
            try:
                bootReasonIndex = self.l3TestConfig.L3.BootReason.index(bootReason)
            except ValueError:
                self.log.error(f"Boot reason {bootReason} not found in configuration")
                return False
        
        # Get reset type index from config
        try:
            resetTypeIndex = self.l3TestConfig.L3.ResetTypes.index(resetType)
        except ValueError:
            self.log.error(f"Reset type {resetType} not found in configuration")
            return False
        
        # Step 1: Set boot reason override using overrideBootReason test
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        self.testcase = "overrideBootReason"
        self.utMenu.start()
        promptWithAnswers = [
            {    
                "query_type": "direct",
                "query": "Enter boot reasons from below options:",
                "input": str(bootReasonIndex)
            }
        ]
        result = self.utMenu.select(BOOT_L3_TEST_SUITE_NAME, self.testcase, promptWithAnswers, self.waitTime, False)
        self.log.info(f"Set boot reason override to {bootReason} with reason string: {bootReason}")
        
        # Step 2: Reboot system with specified reset type
        self.testcase = "rebootSystem"
        promptWithAnswers = [
            {    
                "query_type": "direct",
                "query": "Enter reset types from below options:",
                "input": str(resetTypeIndex)
            }
        ]
        self.log.info(f"Rebooting with reset type: {resetType}")
        self.utMenu.select(BOOT_L3_TEST_SUITE_NAME, self.testcase, promptWithAnswers, self.waitTime, False)
        
        # Wait for reboot
        sleepTime = self.cpe.get("wait_time")
        self.log.info(f"Waiting for {sleepTime} seconds to allow device to reboot...")
        time.sleep(sleepTime)
        self.hal_session.close()
        
        # Step 3: Verify device booted successfully
        result = self.pingTest(logPingTime=True)
        if not result:
            self.log.error(f"Test {testId} FAILED: Device did not boot up successfully")
            return False
        
        # Reinitialize after reboot
        super().__init__(self.testName, "4", self.log)
        
        # Step 4: Check boot reason and verify it matches expected value
        self.testcase = "checkBootReason"
        self.hal_session = self.dut.getConsoleSession("default")
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        self.utMenu.start()
        result = self.utMenu.select(BOOT_L3_TEST_SUITE_NAME, self.testcase, None, self.waitTime, False)
        
        # Extract actual boot reason from result
        pattern = r"bootReason:(?P<boot_reason>[A-Z_]+)"
        match = re.search(pattern, result)
        actualBootReason = None
        if match:
            actualBootReason = match.group("boot_reason")
            self.log.info(f"Actual boot reason after reboot: {actualBootReason}")
        else:
            self.log.error(f"Could not extract boot reason from result")
            return False
        
        # Verify boot reason matches expected
        if actualBootReason == expectedBootReason:
            self.log.stepResult(True, f"Test {testId} PASSED: Boot reason is {actualBootReason} as expected")
            return True
        else:
            self.log.stepResult(False, f"Test {testId} FAILED: Expected {expectedBootReason}, got {actualBootReason}")
            return False

    def testFunction(self):
        """
        The main test function to run the L3 Boot override tests.
        Tests Matrix 2A (override should work) and Matrix 2B (override should be ignored).

        Returns:
            bool: Final result of the test.
        """
        results_2a = []
        results_2b = []
        
        # Run Matrix 2A tests: Override with SOFTWARE_REBOOT (Should Work)
        self.log.stepStart("=" * 80)
        self.log.stepStart("MATRIX 2A: Override with SOFTWARE_REBOOT (Should Work ✅)")
        self.log.stepStart("=" * 80)
        
        if not self.matrix_2a_tests:
            self.log.info("No tests enabled in Matrix 2A (all commented out or empty)")
        
        for test_config in self.matrix_2a_tests:
            # Skip if test_config is None or invalid
            if test_config is None or not isinstance(test_config, dict):
                self.log.error(f"Invalid test configuration found in Matrix 2A: {test_config}")
                continue
            
            try:
                result = self.run_override_test(test_config)
                results_2a.append({"testId": test_config.get("testId", "UNKNOWN"), "passed": result})
            except Exception as e:
                test_id = test_config.get("testId", "UNKNOWN") if isinstance(test_config, dict) else "UNKNOWN"
                self.log.error(f"Exception in test {test_id}: {str(e)}")
                results_2a.append({"testId": test_id, "passed": False})
        
        # Run Matrix 2B tests: Override with Other Reset Types (Should NOT Work)
        self.log.stepStart("=" * 80)
        self.log.stepStart("MATRIX 2B: Override with Other Reset Types (Should NOT Work ❌)")
        self.log.stepStart("=" * 80)
        
        if not self.matrix_2b_tests:
            self.log.info("No tests enabled in Matrix 2B (all commented out or empty)")
        
        for test_config in self.matrix_2b_tests:
            # Skip if test_config is None or invalid
            if test_config is None or not isinstance(test_config, dict):
                self.log.error(f"Invalid test configuration found in Matrix 2B: {test_config}")
                continue
            
            try:
                result = self.run_override_test(test_config)
                results_2b.append({"testId": test_config.get("testId", "UNKNOWN"), "passed": result})
            except Exception as e:
                test_id = test_config.get("testId", "UNKNOWN") if isinstance(test_config, dict) else "UNKNOWN"
                self.log.error(f"Exception in test {test_id}: {str(e)}")
                results_2b.append({"testId": test_id, "passed": False})
        
        # Print summary
        self.log.stepStart("=" * 80)
        self.log.stepStart("TEST SUMMARY")
        self.log.stepStart("=" * 80)
        
        self.log.info("Matrix 2A Results (Override with SOFTWARE_REBOOT):")
        for result in results_2a:
            status = "✅ PASS" if result["passed"] else "❌ FAIL"
            self.log.info(f"  {result['testId']}: {status}")
        
        self.log.info("\nMatrix 2B Results (Override with Other Reset Types - Should Be Ignored):")
        for result in results_2b:
            status = "✅ PASS" if result["passed"] else "❌ FAIL"
            self.log.info(f"  {result['testId']}: {status}")
        
        # Overall result
        all_passed = all([r["passed"] for r in results_2a + results_2b])
        
        if all_passed:
            self.log.stepStart("=" * 80)
            self.log.stepStart("ALL TESTS PASSED ✅")
            self.log.stepStart("=" * 80)
        else:
            self.log.stepStart("=" * 80)
            self.log.stepStart("SOME TESTS FAILED ❌")
            self.log.stepStart("=" * 80)
        
        return all_passed
            
            

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = vts_boot_L3_VerifyOverrideBootReason(summaryLog )
    test.run(False)
