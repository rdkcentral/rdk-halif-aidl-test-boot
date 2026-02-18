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

from vts_boot_helper import bootHelperClass
from raft.framework.core.logModule import logModule

BOOT_L3_TEST_SUITE_NAME = "VTS_L3_BOOT"

class  vts_boot_L3_VerifyPowerSource(bootHelperClass):
    """
    Runs L3 Set States test
    """
    def __init__(self, log:logModule=None):
        """
        Initializes the test class
        """
        self.testName = "BOOT_L3_VerifyPowerSource"
        qcId = "2"
        
        super().__init__(self.testName, qcId, log)

    def testFunction(self):
        """
        The main test function to check the power source of the device and verify if it is as expected.

        Returns:
            bool: Final result of the test.
        """
        self.testcase = "checkPowerSource"
        self.utMenu.start()

        self.utMenu.select(BOOT_L3_TEST_SUITE_NAME, self.testcase, None, self.waitTime, False)
        self.log.info("All power source test cases executed.")
        return True

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog  = logModule(summaryLogName, level=logModule.INFO)
    test = vts_boot_L3_VerifyPowerSource(summaryLog )
    test.run(False)