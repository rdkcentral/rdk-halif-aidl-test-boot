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

#include "com/rdk/hal/bootreason/IBootReason.h"
#include <iostream>

#define BOOT_L3_TEST_CONFIG_FILE "test_configuration/vts_l3_boot.yaml"

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
        UT_LOG_DEBUG("Capabilities supportedBootCauses size: %zu", capabilities.supportedBootCauses.size()); \
     }while (0)

// Macro for getBootReason - positive case
#define BOOT_GETBOOTREASON_SUCCESS(bootHandle, bootReason)                 \
    do{                                                                           \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getBootCause(OUT:bootCause)"); \
        status = bootHandle->getBootCause(&bootReason); \
        UT_LOG_DEBUG("Result getBootCause(OUT:bootCause), status:[isOK:%d, exception:%d, bootCause:%s]", status.isOk(), status.exceptionCode(), com::rdk::hal::bootreason::toString(bootReason).c_str()); \
        UT_ASSERT_TRUE(status.isOk());                                        \
        UT_ASSERT_EQUAL(status.exceptionCode(), ::android::binder::Status::EX_NONE); \
     }while (0)

// Macro for reboot - positive case
#define BOOT_REBOOT_SUCCESS(bootHandle, resetTypes, reasonString)                 \
    do{                                                                           \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling reboot(IN:resetTypes[%s], reasonString[%s])", com::rdk::hal::bootreason::toString(resetTypes).c_str(), ::android::String8(reasonString).c_str()); \
        fflush(stdout); \
        fflush(stderr); \
        sleep(1); /* Sleep for a short duration to allow log to be printed before reboot happens */ \
        status = bootHandle->reboot(resetTypes, reasonString); \
        UT_LOG_DEBUG("Result set(OUT:), status:[isOK:%s, exception:%s]", status.isOk() ? "true" : "false", std::to_string(status.exceptionCode()).c_str()); \
        UT_ASSERT_TRUE(status.isOk());                                        \
        UT_ASSERT_EQUAL(status.exceptionCode(), ::android::binder::Status::EX_NONE); \
     }while (0)

// Macro for reboot - negative case
#define BOOT_REBOOT_FAILURE(bootHandle, resetTypes, reasonString, expectedException)                 \
    do{                                                                           \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling reboot(IN:resetTypes[%s], reasonString[%s])", com::rdk::hal::bootreason::toString(resetTypes).c_str(), ::android::String8(reasonString).c_str()); \
        sleep(1); /* Sleep for a short duration to allow log to be printed before reboot happens */ \
        status = bootHandle->reboot(resetTypes, reasonString); \
        UT_LOG_DEBUG("Result set(OUT:), status:[isOK:%s, exception:%s]", status.isOk() ? "true" : "false", std::to_string(status.exceptionCode()).c_str()); \
        UT_ASSERT_FALSE(status.isOk());                                        \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
     }while (0)

// Macro for getPowerSource - positive case
#define BOOT_GETPOWERSOURCE_SUCCESS(bootHandle, powerSource)                 \
    do{                                                                           \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getPowerSource(OUT:powerSource)"); \
        status = bootHandle->getPowerSource(&powerSource); \
        UT_LOG_DEBUG("Result getPowerSource(OUT:powerSource), status:[isOK:%d, exception:%d, powerSource:%s]", status.isOk(), status.exceptionCode(), com::rdk::hal::bootreason::toString(powerSource).c_str()); \
        UT_ASSERT_TRUE(status.isOk());                                        \
        UT_ASSERT_EQUAL(status.exceptionCode(), ::android::binder::Status::EX_NONE); \
     }while (0)

// Macro for setBootReason - positive case
#define BOOT_SETBOOTREASON_SUCCESS(bootHandle, bootReason, reasonString)                 \
    do{                                                                           \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling setBootCause(IN:bootCause[%s], reasonString[%s])", com::rdk::hal::bootreason::toString(bootReason).c_str(), reasonString.c_str()); \
        status = bootHandle->setBootCause(bootReason, ::android::String16(reasonString.c_str())); \
        UT_LOG_DEBUG("Result setBootCause(OUT:), status:[isOK:%s, exception:%s]", status.isOk() ? "true" : "false", std::to_string(status.exceptionCode()).c_str()); \
        UT_ASSERT_TRUE(status.isOk());                                        \
        UT_ASSERT_EQUAL(status.exceptionCode(), ::android::binder::Status::EX_NONE); \
     }while (0)

// Macro for setBootReason - negative case
#define BOOT_SETBOOTREASON_FAILURE(bootHandle, bootReason, reasonString, expectedException) \
    do                                                                                      \
    {                                                                                       \
        ::android::binder::Status status;                                                   \
        const char *_rs = (reasonString && reasonString[0] != '\0')                         \
                      ? reasonString                                                \
                      : "NULL";                                                     \
        ::android::String16 reasonStr16 = reasonString ? ::android::String16(reasonString) : ::android::String16(); \
        UT_LOG_DEBUG("Calling setBootCause(IN:bootCause[%s], reasonString[%s])", com::rdk::hal::bootreason::toString(bootReason).c_str(), _rs); \
        status = bootHandle->setBootCause(bootReason, reasonStr16); \
        UT_LOG_DEBUG("Result setBootCause(OUT:), status:[isOK:%s, exception:%d]", status.isOk() ? "true" : "false", status.exceptionCode()); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
    }while (0)

// Macro to create a kvp instance from a file
#define BOOT_KVP_CREATE_INSTANCE(instance, fileName)                              \
    do{                                                                            \
        ut_kvp_status_t result;                                                    \
        instance = ut_kvp_createInstance();                                        \
        if (!instance)                                                             \
        {                                                                          \
            UT_LOG_ERROR("[BOOT]Failed to create ut kvp instance");                \
            break;                                                                 \
        }                                                                          \
        result = ut_kvp_open(instance, const_cast<char*>(fileName));               \
        if (result != UT_KVP_STATUS_SUCCESS)                                       \
        {                                                                          \
            UT_LOG_ERROR("[BOOT]Failed to open test configuration file");          \
            ut_kvp_destroyInstance(instance);                                      \
            instance = nullptr;                                                    \
            break;                                                                 \
        }                                                                          \
    }while (0)

// Macro to destroy a kvp instance
#define BOOT_KVP_DESTROY_INSTANCE(instance)                                       \
    do{                                                                            \
        ut_kvp_destroyInstance((ut_kvp_instance_t*)instance);                      \
    }while (0)

/**
 * @} // End of BOOT_HALTEST_CLASS
 * @} // End of BOOT_HALTEST
 * @} // End of BOOT
 * @} // End of HPK
 */
/* End of vts_boot.h */