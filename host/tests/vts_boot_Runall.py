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
import importlib
from pathlib import Path

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))

from raft.framework.plugins.ut_raft.configRead import ConfigRead
from raft.framework.core.logModule import logModule


def Runall_L3():
    skipTests = []

    summary_log_name = f"{os.path.splitext(os.path.basename(__file__))[0]}_summary"
    summaryLogL3 = logModule(summary_log_name, level=logModule.INFO)

    test_directory = Path(dir_path)

    # Find all test modules in the directory
    test_modules = sorted(test_directory.glob("vts_boot_L3_*.py"))
    summaryLogL3.info(f"Found {len(test_modules)} L3 test modules to run.")

    # Run each test by dynamically importing and instantiating
    for test_module_path in test_modules:
        # Construct module name from file name, excluding .py extension
        module_name = test_module_path.stem
        skip = False
        for skipTest in skipTests:
            if skipTest in module_name:
                skip = True
                break
        if skip:
            continue
        try:
            # Dynamically import the module
            module = importlib.import_module(module_name)
            summaryLogL3.info(f"Running test module: {module_name}")

            # Dynamically access the test class from the module
            # Assuming each test file has only one class named the same as the module
            test_class = getattr(module, module_name)

            # Instantiate and run the test
            test_instance = test_class(summaryLogL3)
            test_instance.run(False)

        except (ImportError, AttributeError) as e:
            summaryLogL3.error(f"Failed to import {module_name}: {e}")
def Run_L1L2():
    summaryLogL1L2 = logModule("vts_boot_L1L2", level=logModule.INFO)
    test_module_name = "vts_boot_L1_L2_tests"
    try:
        module = importlib.import_module(test_module_name)
        test_class = getattr(module, "vts_boot_L1_L2_tests")
        test_instance = test_class(summaryLogL1L2)
        test_instance.run(False)
    except (ImportError, AttributeError) as e:
        summaryLogL1L2.error(f"Failed to import {test_module_name}: {e}")

if __name__ == '__main__':
    Run_L1L2()
    Runall_L3()