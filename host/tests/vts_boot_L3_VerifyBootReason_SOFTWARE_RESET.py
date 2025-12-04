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
from raft.framework.core.utPlaneController import utPlaneController
from raft.framework.plugins.ut_raft import utHelperClass
from raft.framework.plugins.ut_raft.utSuiteNavigator import UTSuiteNavigatorClass
from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils
from vts_boot_helper import bootHelperClass

class  vts_boot_L3_VerifyBootReason_SOFTWARE_RESET(bootHelperClass):
    """
    Runs L3 Set States test
    """
    def __init__(self, log:logModule=None):
        """
        Initializes the test class
        """
        self.testName = "vts_boot_L3_VerifyBootReason_SOFTWARE_RESET"
        qcId = "1"
        
        super().__init__(self.testName, qcId, log)

    def testFunction(self):
        """
        The main test function to run the L3 Boot reboot with boot reason SOFTWARE_RESET and verify it.

        Returns:
            bool: Final result of the test.
        """
        bootReasonExpected = "SOFTWARE_RESET"
        self.rebootWithBootReason(expectedBootReason=bootReasonExpected)

        super().__init__(self.testName, "1", self.log) #Reinitialize after reboot

        self.runTestCaseWithBootReason(bootReasonExpected)

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summery"
    summaryLog  = logModule(summaryLogName, level=logModule.INFO)
    test = vts_boot_L3_VerifyBootReason_SOFTWARE_RESET(summaryLog )
    test.run(False)
