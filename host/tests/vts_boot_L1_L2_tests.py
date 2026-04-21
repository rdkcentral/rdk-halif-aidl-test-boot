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

BOOT_L1_TEST_SUITE_NAME = "VTS_L1_BOOT"
BOOT_L2_TEST_SUITE_NAME = "VTS_L2_BOOT"
BOOT_REASON_FILE_ON_DEVICE = "/var/boot_reason.txt"
RESET_TYPE_FILE_ON_DEVICE = "/var/reset_type.txt"
POWER_SOURCE_FILE_ON_DEVICE = "/var/power_source.txt"

class  vts_boot_L1_L2_tests(bootHelperClass):
    """
    Runs automated L1, L2 BOOT tests on the target device.
    Handles setup, service restarts, asset management.
    Collects and aggregates test results for pass/fail reporting.
    """
    def __init__(self, log:logModule=None):
        """
        Initializes the test class
        """
        qcId = "1"
        self.testName = "boot_L1_L2"
        #configureNetwork = False
        super().__init__(self.testName, qcId, log)

    def runTest(self, testSuite, test_case:str=None):
        """
        Runs the test case passed to this funtion
        Args:
            test_case (str, optional): test case name to run, default runs all test
        Returns:
            bool: True - test pass, False - test fails
        """
        testsuite_name = testSuite.get("name")

        output = self.utMenu.select(testsuite_name, test_case, None, self.waitTime, False)
        results = self.utMenu.collect_results(output, True)
        if results is None:
            results = False
        return results

    def testFunction(self):
        """
        The main test function to run the L1 and L2 tests.

        Returns:
            bool: Final result of the test.
        """
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        if self.platform == "vDevice":
                self.removeFilesOnDevice([BOOT_REASON_FILE_ON_DEVICE, RESET_TYPE_FILE_ON_DEVICE, POWER_SOURCE_FILE_ON_DEVICE])

        self.utMenu.start()

        testsuites = self.testCaseConfiguration.fields.get('test_suites')
        finalresult = True

        for testsuite in testsuites:
            testsuite_name = testsuite.get("name")
            if( testsuite_name != BOOT_L1_TEST_SUITE_NAME and testsuite_name != BOOT_L2_TEST_SUITE_NAME):
                continue

            test_cases = testsuite.get("test_cases")

            if len(test_cases) == 1 and test_cases[0] == "all":
                self.log.stepStart(f'Test Suite: {testsuite_name} Run all Tests cases')
                # If 'all' test case mentioned in list, run all tests with 'r' option
                result = self.runTest(testsuite)
                if not result:
                    finalresult = False
                self.log.stepResult(result, f'Test Suit: {testsuite_name} Test Case: all')
            else:
                for test_case in testsuite.get("test_cases"):
                    self.log.stepStart(f'Test Suite: {testsuite_name} Test Case: {test_case}')
                    result = self.runTest(testsuite, test_case)
                    if not result:
                        finalresult = False
                    self.log.stepResult(result, f'Test Suit: {testsuite_name} Test Case: {test_case}')
        return finalresult

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = vts_boot_L1_L2_tests(summaryLog)
    test.run(False)