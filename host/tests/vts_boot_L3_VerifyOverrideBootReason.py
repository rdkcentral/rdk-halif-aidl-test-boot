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

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))

from vts_boot_helper import bootHelperClass
from raft.framework.core.logModule import logModule

BOOT_L3_TEST_SUITE_NAME = "VTS_L3_BOOT"

class vts_boot_L3_VerifyOverrideBootReason(bootHelperClass):
    """
    Runs L3 test to verify boot reason override behavior.
    Tests that:
    - SOFTWARE_REBOOT preserves the override
    - Other reset types ignore the override
    """
    def __init__(self, log:logModule=None, testMenuConfigFile:str=None, moduleName:str=None, artifactDirPath:str=None):
        """
        Initializes the test class
        """
        self.testName = "BOOT_L3_OverrideBootReason"
        qcId = "4"
        super().__init__(self.testName, qcId, log, testMenuConfigFile, moduleName, artifactDirPath)

    def testOverrideWithResetType(self, boot_reason: str, reset_type: str, test_id: str) -> bool:
        """
        Test boot reason override with a specific reset type.
        
        Args:
            boot_reason (str): Boot reason to override with
            reset_type (str): Reset type to use for reboot
            test_id (str): Test identifier for logging
            
        Returns:
            bool: True if test passed, False otherwise
        """
        self.log.stepStart(f"Test {test_id}: Override={boot_reason}, ResetType={reset_type}")
        
        # Get indices from configuration
        try:
            bootReasonIndex = self.l3TestConfig.L3.BootCause.index(boot_reason)
            resetTypeIndex = self.l3TestConfig.L3.ResetTypes.index(reset_type)
        except ValueError as e:
            self.log.error(f"Configuration error: {e}")
            return False
        
        # Determine expected boot reason based on reset type
        # SOFTWARE_REBOOT preserves override, all others ignore it
        if reset_type == "SOFTWARE_REBOOT":
            expected_boot_reason = boot_reason
            self.log.info(f"SOFTWARE_REBOOT: Override should be preserved → expect {expected_boot_reason}")
        else:
            # For other reset types, override is ignored
            # Expected boot reason depends on the reset type itself
            if reset_type == "MAINTENANCE_REBOOT":
                expected_boot_reason = "MAINTENANCE_REBOOT"
            elif reset_type == "FULL_SYSTEM_RESET":
                expected_boot_reason = "WARM_RESET"
            elif reset_type == "INVALIDATE_CURRENT_APPLICATION_IMAGE":
                expected_boot_reason = "WARM_RESET"
            elif reset_type == "FORCE_DISASTER_RECOVERY":
                expected_boot_reason = "WARM_RESET"
            else:
                expected_boot_reason = "WARM_RESET"
            self.log.info(f"{reset_type}: Override should be ignored → expect {expected_boot_reason}")
        
        # Step 1: Set boot reason override
        if not self.overrideBootReason(BOOT_L3_TEST_SUITE_NAME, bootReasonIndex, boot_reason):
            self.log.stepResult(False, f"Test Suit: {self.testName} Test Case: {test_id}")
            return False
        
        # Step 2: Reboot with specified reset type
        if not self.performAndVerifyReboot(BOOT_L3_TEST_SUITE_NAME, resetTypeIndex, reset_type):
            self.log.stepResult(False, f"Test Suit: {self.testName} Test Case: {test_id}")
            return False
        
        # Reinitialize after reboot
        super().__init__(self.testName, "4", self.log, self._testMenuConfigFile, self._moduleName, self._artifactDirPath)
        
        # Step 3: Verify boot reason
        return self.verifyBootReason(reset_type, expected_boot_reason, test_id)

    def testFunction(self):
        """
        Main test function. Tests override behavior with different reset types.
        
        Returns:
            bool: True if all tests passed, False otherwise
        """
        results = []
        
        self.log.stepStart("=" * 80)
        self.log.stepStart("BOOT REASON OVERRIDE TESTS")
        self.log.stepStart("=" * 80)
        
        # Test 1: Override with SOFTWARE_REBOOT (should preserve override)
        self.log.info("Test Scenario: Override should be preserved with SOFTWARE_REBOOT")
        result1 = self.testOverrideWithResetType("WATCHDOG", "SOFTWARE_REBOOT", "OVR-01")
        results.append({"test": "OVR-01: WATCHDOG + SOFTWARE_REBOOT", "passed": result1})
        
        # Test 2: Override with MAINTENANCE_REBOOT (should ignore override)
        self.log.info("Test Scenario: Override should be ignored with MAINTENANCE_REBOOT")
        result2 = self.testOverrideWithResetType("WATCHDOG", "MAINTENANCE_REBOOT", "OVR-02")
        results.append({"test": "OVR-02: WATCHDOG + MAINTENANCE_REBOOT", "passed": result2})
        
        # Test 3: Override with FULL_SYSTEM_RESET (should ignore override)
        self.log.info("Test Scenario: Override should be ignored with FULL_SYSTEM_RESET")
        result3 = self.testOverrideWithResetType("THERMAL_RESET", "FULL_SYSTEM_RESET", "OVR-03")
        results.append({"test": "OVR-03: THERMAL_RESET + FULL_SYSTEM_RESET", "passed": result3})
        
        # Test 4: Override with INVALIDATE_CURRENT_APPLICATION_IMAGE (should ignore override)
        self.log.info("Test Scenario: Override should be ignored with INVALIDATE_CURRENT_APPLICATION_IMAGE")
        result4 = self.testOverrideWithResetType("COLD_BOOT", "INVALIDATE_CURRENT_APPLICATION_IMAGE", "OVR-04")
        results.append({"test": "OVR-04: COLD_BOOT + INVALIDATE_CURRENT_APPLICATION_IMAGE", "passed": result4})
        
        # Test 5: Override with FORCE_DISASTER_RECOVERY (should ignore override)
        self.log.info("Test Scenario: Override should be ignored with FORCE_DISASTER_RECOVERY")
        result5 = self.testOverrideWithResetType("STR_AUTH_FAILURE", "FORCE_DISASTER_RECOVERY", "OVR-05")
        results.append({"test": "OVR-05: STR_AUTH_FAILURE + FORCE_DISASTER_RECOVERY", "passed": result5})
        
        # Print summary
        self.log.stepStart("=" * 80)
        self.log.stepStart("TEST SUMMARY")
        self.log.stepStart("=" * 80)
        
        for result in results:
            status = "✅ PASS" if result["passed"] else "❌ FAIL"
            self.log.info(f"  {result['test']}: {status}")
        
        # Overall result
        all_passed = all([r["passed"] for r in results])
        
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

VTS_BOOT_L3_VerifyOverrideBootReason = vts_boot_L3_VerifyOverrideBootReason
