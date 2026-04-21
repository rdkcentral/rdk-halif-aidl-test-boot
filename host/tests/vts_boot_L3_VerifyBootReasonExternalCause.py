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

class vts_boot_L3_VerifyBootReasonExternalCause(bootHelperClass):
    """
    Runs L3 test to verify boot reason after reboot from external cause.
    Tests that the device correctly identifies and reports boot reasons triggered by external events.
    """
    def __init__(self, log:logModule=None):
        """
        Initializes the test class
        """
        self.testName = "BOOT_L3_VerifyBootReasonExternalCause"
        qcId = "1"
        
        super().__init__(self.testName, qcId, log)

    def testFunction(self):
        """
        Main test function to verify boot reasons after reboot from external cause.
        Triggers reboots with each boot reason from config and verifies the device correctly reports it.

        Returns:
            bool: Final result of the test.
        """
        totatlResult = True
        for bootReason in self.l3TestConfig.L3.BootReason:
            bootReasonExpected = bootReason
            self.log.stepStart(f"Starting test for boot reason: {bootReasonExpected}")
            boot_result = self.externallyTriggeredReboot(expectedBootReason=bootReasonExpected)
            self.log.info(f"Reboot done with boot reason: {bootReasonExpected}")
            super().__init__(self.testName, "1", self.log) #Reinitialize after reboot
            test_result = self.verifyBootReason("external cause", bootReasonExpected)
            totatlResult = totatlResult and bool(boot_result) and bool(test_result)
            self.log.info(f"Boot reason '{bootReasonExpected}' test completed.\n")
        self.log.info("All boot reason test cases executed.")
        return totatlResult

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = vts_boot_L3_VerifyBootReasonExternalCause(summaryLog)
    test.run(False)
