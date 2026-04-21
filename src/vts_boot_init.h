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
 * @addtogroup BOOT_HALTEST BOOT HAL Tests
 * @{
 */

/**
 * @defgroup BOOT_HALTEST_CLASS BOOT HAL Tests initialization Header File
 * @{
 * @parblock
 *
 * ### Common functions/macros required for the BOOT module :
 *
 * Common functions/macros required for the BOOT module .
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_boot_init.h
 *
 */

#pragma once
#include <ut.h>

UT_status_t VTS_BOOT_init();
void VTS_BOOT_destroy();

 /**
 * @} // End of BOOT_HALTEST_CLASS
 * @} // End of BOOT_HALTEST
 * @} // End of BOOT
 * @} // End of HPK
 */
/* End of vts_boot_init.h */