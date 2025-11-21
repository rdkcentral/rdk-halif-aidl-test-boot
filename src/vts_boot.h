/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
 * @addtogroup  Boot Module
 * @{
 */

/**
 * @addtogroup BOOT_HALTEST BOOT HAL Tests
 * @{
 */

/**
 * @defgroup BOOT_HALTEST_CLASS BOOT HAL Header File
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
 * @file vts_boot.h
 *
 */

#pragma once

#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>

#include "com/rdk/hal/boot/IBoot.h"

// Macro for getCapabilities - positive case
#define BOOT_GETCAPABILITIES_SUCCESS(bootHandle, capabilities)                 \
    do{                                                                           \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getCapabilities(OUT:caps[])"); \
        status = bootHandle->getCapabilities(&capabilities); \
        UT_LOG_DEBUG("Result getCapabilities(OUT:caps), status:[isOK:%d, exception:%d]", status.isOk(), status.exceptionCode()); \
        UT_ASSERT_TRUE(status.isOk());                                        \
        UT_ASSERT_EQUAL(status.exceptionCode(), ::android::binder::Status::EX_NONE); \
        UT_LOG_DEBUG("Capabilities supportedResetTypes size: %zu", capabilities.supportedResetTypes.size()); \
        UT_LOG_DEBUG("Capabilities supportedBootReasons size: %zu", capabilities.supportedBootReasons.size()); \
     }while (0)

// Macro for getBootReason - positive case
#define BOOT_GETBOOTREASON_SUCCESS(bootHandle, bootReason)                 \
    do{                                                                           \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getBootReason(OUT:bootReason)"); \
        status = bootHandle->getBootReason(&bootReason); \
        UT_LOG_DEBUG("Result getBootReason(OUT:bootReason), status:[isOK:%d, exception:%d]", status.isOk(), status.exceptionCode()); \
        UT_ASSERT_TRUE(status.isOk());                                        \
        UT_ASSERT_EQUAL(status.exceptionCode(), ::android::binder::Status::EX_NONE); \
     }while (0)

// Macro for reboot - positive case
#define BOOT_REBOOT_SUCCESS(bootHandle, resetTypes)                 \
    do{                                                                           \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling reboot(IN:resetTypes[%d])", resetTypes); \
        status = bootHandle->reboot(resetTypes); \
        UT_LOG_DEBUG("Result set(OUT:), status:[isOK:%s, exception:%s]", status.isOk() ? "true" : "false", std::to_string(status.exceptionCode()).c_str()); \
        UT_ASSERT_TRUE(status.isOk());                                        \
        UT_ASSERT_EQUAL(status.exceptionCode(), ::android::binder::Status::EX_NONE); \
     }while (0)

// Macro for shutdown - positive case
#define BOOT_SHUTDOWN_SUCCESS(bootHandle, result)                 \
    do{                                                                           \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling shutdown(OUT:result[])"); \
        result = ""; \
        status = bootHandle->shutdown(&result); \
        UT_LOG_DEBUG("Result set(OUT:%s), status:[isOK:%s, exception:%s]", result.c_str(), status.isOk() ? "true" : "false", std::to_string(status.exceptionCode()).c_str()); \
        UT_ASSERT_TRUE(status.isOk());                                        \
        UT_ASSERT_EQUAL(status.exceptionCode(), ::android::binder::Status::EX_NONE); \
     }while (0)

// Macro for reboot - negative case
#define BOOT_REBOOT_FAILURE(bootHandle, resetTypes, expectedException)                 \
    do{                                                                           \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling reboot(IN:resetTypes[%d])", resetTypes); \
        status = bootHandle->reboot(resetTypes); \
        UT_LOG_DEBUG("Result set(OUT:), status:[isOK:%s, exception:%s]", status.isOk() ? "true" : "false", std::to_string(status.exceptionCode()).c_str()); \
        UT_ASSERT_FALSE(status.isOk());                                        \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
     }while (0)


/**
 * @} // End of BOOT_HALTEST_CLASS
 * @} // End of BOOT_HALTEST
 * @} // End of BOOT
 * @} // End of HPK
 */
/* End of vts_boot.h */