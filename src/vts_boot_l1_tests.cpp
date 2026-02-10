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
 * @addtogroup BOOT Module
 * @{
 */

/**
 * @addtogroup BOOT_HALTEST BOOT HAL Tests
 * @{
 */

/**
 * @defgroup BOOT_HALTEST_CLASS BOOT HAL Tests L1 Test Cases
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
 * @file vts_boot_l1_tests.cpp
 * @brief L1 Test Cases for the BOOT HAL (Hardware Abstraction Layer) interface.
 *
 * This file contains unit tests for the BOOT module as part of the Hardware Porting Kit (HPK).
 * The tests validate the correct implementation of the IBoot interface, including its capabilities,
 * versioning, and property handling functionalities.
 *
 * @addtogroup HPK Hardware */


#include <ut_log.h>
#include <ut_kvp_profile.h>

#include "vts_boot.h"

extern android::sp<com::rdk::hal::boot::IBoot> gBOOTHandle;

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L1_BOOT, UT_TESTS_L1)

class VTS_L1_BOOT : public UTCore
{
public:
    VTS_L1_BOOT() : UTCore() {}

    ~VTS_L1_BOOT() override = default;

    void SetUp() override
    {
        android::ProcessState::self()->startThreadPool();
    }

    void TearDown() override
    {
    }
};

// Test case to validate getCapabilities() method of boot interface, positive case
UT_ADD_TEST(VTS_L1_BOOT, getCapabilities_pos)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::boot::Capabilities capabilities;
    BOOT_GETCAPABILITIES_SUCCESS(bootHandle, capabilities);
    const std::string baseKey = "boot/supportedResetTypes";
    UT_ASSERT_EQUAL(capabilities.supportedResetTypes.size(), UT_KVP_PROFILE_GET_LIST_COUNT(baseKey.c_str()));
    bool found = false;
    for (const auto &resetTypes : capabilities.supportedResetTypes)
    {
        const std::string resetTypesStr = android::internal::ToString(resetTypes);
        UT_LOG_DEBUG("Supported resetTypesStr: %s\n", resetTypesStr.c_str());
        std::string key;
        for (int i = 0; i < (int)capabilities.supportedResetTypes.size(); i++)
        {
            key = baseKey + "/" + std::to_string(i);
            char value[UT_KVP_MAX_ELEMENT_SIZE] = {0};
            if (ut_kvp_getStringField(ut_kvp_profile_getInstance(), key.c_str(), value, sizeof(value)) == UT_KVP_STATUS_SUCCESS)
            {
                if (strcmp(value, resetTypesStr.c_str()) == 0) // Validate that the reset type string in profile matches with the reset type string from capabilities
                {
                    found = true;
                }
            }
        }
    }
    UT_ASSERT_TRUE(found); // Validate that each reset type from capabilities is present in profile

    const std::string baseKeyBootReasons = "boot/supportedBootReasons";
    UT_ASSERT_EQUAL(capabilities.supportedBootReasons.size(), UT_KVP_PROFILE_GET_LIST_COUNT(baseKeyBootReasons.c_str()));
    found = false;
    for (const auto &bootReason : capabilities.supportedBootReasons)
    {
        const std::string bootReasonStr = android::internal::ToString(bootReason);
        UT_LOG_DEBUG("Supported bootReasonStr: %s\n", bootReasonStr.c_str());
        std::string key;
        for (int i = 0; i < (int)capabilities.supportedBootReasons.size(); i++)
        {
            key = baseKeyBootReasons + "/" + std::to_string(i);
            char value[UT_KVP_MAX_ELEMENT_SIZE] = {0};
            if (ut_kvp_getStringField(ut_kvp_profile_getInstance(), key.c_str(), value, sizeof(value)) == UT_KVP_STATUS_SUCCESS)
            {
                if (strcmp(value, bootReasonStr.c_str()) == 0) // Validate that the boot reason string in profile matches with the boot reason string from capabilities
                {
                    found = true;
                }
            }
        }
    }
    UT_ASSERT_TRUE(found); // Validate that each boot reason from capabilities is present in profile
}

// Test case to validate setBootReason() method of boot interface, positive case
UT_ADD_TEST(VTS_L1_BOOT, setBootReason_pos)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::boot::BootReason bootReasonToSet = com::rdk::hal::boot::BootReason::ERROR_UNKNOWN;
    std::string reasonString = com::rdk::hal::boot::toString(bootReasonToSet);
    BOOT_SETBOOTREASON_SUCCESS(bootHandle, bootReasonToSet, reasonString);
}

// Test case to validate setBootReason() method of boot interface, negative case with unsupported boot reason
UT_ADD_TEST(VTS_L1_BOOT, setBootReason_neg)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);

    // Using an unsupported boot reason to test negative case
    com::rdk::hal::boot::BootReason invalidBootReason = static_cast<com::rdk::hal::boot::BootReason>(-999);
    std::string reasonString = com::rdk::hal::boot::toString(invalidBootReason);
    BOOT_SETBOOTREASON_FAILURE(bootHandle, invalidBootReason, reasonString.c_str(), ::android::binder::Status::EX_ILLEGAL_ARGUMENT);

    //Using empty reason string to test negative case
    BOOT_SETBOOTREASON_FAILURE(bootHandle, com::rdk::hal::boot::BootReason::ERROR_UNKNOWN, "", ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

// Test case to validate getBootReason() method of boot interface, positive case
UT_ADD_TEST(VTS_L1_BOOT, getBootReason_pos)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::boot::BootReason bootReason;
    BOOT_GETBOOTREASON_SUCCESS(bootHandle, bootReason);

    // Validate that the returned boot reason is among the supported boot reasons in capabilities
    com::rdk::hal::boot::Capabilities capabilities;
    bool reasonMatched = false;
    BOOT_GETCAPABILITIES_SUCCESS(bootHandle, capabilities);
    for(const auto &reason : capabilities.supportedBootReasons)
    {
        if(bootReason == reason)
        {
            UT_PASS("Boot reason from getBootReason() is among supported boot reasons in capabilities");
            reasonMatched = true;
            break;
        }
    }
    UT_ASSERT_TRUE(reasonMatched); // Validate that the returned boot reason is among supported boot reasons in capabilities
}

// Test case to validate reboot() method of boot interface, negative case with unsupported reset type
UT_ADD_TEST(VTS_L1_BOOT, reboot_neg)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    // Using an unsupported reset type to test negative case
    com::rdk::hal::boot::ResetType invalidResetType = static_cast<com::rdk::hal::boot::ResetType>(-999);
    ::android::String16 reasonString = android::String16("invalidResetType");
    BOOT_REBOOT_FAILURE(bootHandle, invalidResetType, reasonString, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);

    // Using empty reason string to test negative case
    BOOT_REBOOT_FAILURE(bootHandle, com::rdk::hal::boot::ResetType::FULL_SYSTEM_RESET, android::String16(""), ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

// Test case to validate getPowerSource() method of boot interface, positive case
UT_ADD_TEST(VTS_L1_BOOT, getPowerSource_pos)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::boot::PowerSource powerSource;
    BOOT_GETPOWERSOURCE_SUCCESS(bootHandle, powerSource);

    // Validate that the returned power source matches the supported power sources in profile
    const std::string baseKey = "boot/supportedPowerSources";
    bool powerSourceMatched = false;
    for (int32_t i = 0; i < UT_KVP_PROFILE_GET_LIST_COUNT(baseKey.c_str()); i++)
    {
        char value[UT_KVP_MAX_ELEMENT_SIZE] = {0};
        std::string key = baseKey + "/" + std::to_string(i);
        if (ut_kvp_getStringField(ut_kvp_profile_getInstance(), key.c_str(), value, sizeof(value)) == UT_KVP_STATUS_SUCCESS)
        {
            if (strcmp(value, android::internal::ToString(powerSource).c_str()) == 0) // Validate that the power source string in profile matches with the power source string from getPowerSource()
            {
                UT_PASS("Power source from getPowerSource() is among supported power sources in profile");
                powerSourceMatched = true;
                break;
            }
        }
    }
    UT_ASSERT_TRUE(powerSourceMatched); // Validate that the returned power source matches the supported power sources in profile
}





/**
* @} // End of BOOT_HALTEST_CLASS
* @} // End of BOOT_HALTEST
* @} // End of BOOT
* @} // End of HPK
*/
/* End of vts_boot_l1_tests.cpp */