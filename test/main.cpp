/*
 *  If not stated otherwise in this file or this component's LICENSE
 *  file the following copyright and licenses apply:
 *
 *  Copyright 2026 RDK Management
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @addtogroup HPK Hardware Porting Kit
 * @{
 * @par The Hardware Porting Kit
 * HPK is the next evolution of the well-defined Hardware Abstraction Layer
 * (HAL), but augmented with more comprehensive documentation and test suites
 * that OEM or SOC vendors can use to self-certify their ports before taking
 * them to RDKM for validation or to an operator for final integration and
 * deployment. The Hardware Porting Kit effectively enables an OEM and/or SOC
 * vendor to self-certify their own Video Accelerator devices, with minimal RDKM
 * assistance.
 *
 */

/**
 * @addtogroup BOOT Module
 * @{
 */

/**
 * @defgroup BOOT_HALTEST BOOT HAL Tests
 * @{
 */

/**
 * @defgroup BOOT_HALTEST_MAIN BOOT HAL Tests Main File
 *  @{
 * @parblock
 *
 * ### Tests for BOOTHAL :
 *
 * This is to ensure that the API meets the operational requirements of the module across all vendors.
 *
 * **Pre-Conditions:** None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 */

/**
 * @file main.cpp
 *
 */

#include <ut.h>
#include <ut_log.h>
#include "vts_boot_init.h"

int main(int argc, char **argv)
{
    UT_status_t result;

    UT_init(argc, argv);

    UT_LOG_INFO("==========================================================================");
    UT_LOG_INFO("\t BUILD_TIMESTAMP: \033[0;32m%s\033[0m", BUILD_TIMESTAMP);
    UT_LOG_INFO("\t UNIFIED VTS Version: \033[0;32m%s\033[0m", HALIF_TEST_PROJECT_VERSION);
    UT_LOG_INFO("\t AIDL HALIF Version: \033[0;32m%s\033[0m", RDK_HALIF_AIDL_VERSION);
    UT_LOG_INFO("\t VTS UNIFIED BOOT HALIF Test Version: \033[0;32m%s\033[0m", BOOT_VERSION);
    UT_LOG_INFO("==========================================================================");

    result = VTS_BOOT_init();
    if (result != UT_STATUS_OK)
    {
        UT_LOG_ERROR("VTS_BOOT_init() failed!");
        return result;
    }

    result = UT_run_tests();

    VTS_BOOT_destroy();

    return result;
}

/** @} */ // End of BOOT_HALTEST_MAIN
/** @} */ // End of BOOT_HALTEST
/** @} */ // End of BOOT Module
/** @} */ // End of HPK
