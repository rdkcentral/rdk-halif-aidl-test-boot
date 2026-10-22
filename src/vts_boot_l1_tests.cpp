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

constexpr int INVALID_ENUM_TEST_VALUE = -999; // An invalid enum value to test negative cases
extern android::sp<com::rdk::hal::bootreason::IBootReason> gBOOTHandle;

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

/**
 * @brief Positive test for getCapabilities() function of boot interface
 *
 * This test verifies that the getCapabilities() method of the boot interface
 * correctly populates the Capabilities structure with the supported boot reasons
 * and reset types as defined in the configuration profile.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 001@n
 *
 * **Test Procedure:**
 * | Step | Action                                          | Input / Test Data                                       | Expected Result                                    | Notes                                      |
 * |------|-------------------------------------------------|---------------------------------------------------------|----------------------------------------------------|-----------------------------------------|
 * | 1    | Obtain boot handle                              | gBOOTHandle                                             | bootHandle is not nullptr                          |                                            |
 * | 2    | Call BOOT_GETCAPABILITIES_SUCCESS               | bootHandle, capabilities                                | capabilities structure is populated                |                                            |
 * | 3    | Compare supportedResetTypes count               | capabilities.supportedResetTypes, UT_KVP_PROFILE_GET_LIST_COUNT | Counts are equal                                   |                                            |
 * | 4    | For each reset type, validate against profile   | resetTypes, ut_kvp_getStringField                       | Each reset type is present in the profile          | Checks each supported reset type           |
 * | 5    | Compare supportedBootCauses count               | capabilities.supportedBootCauses, UT_KVP_PROFILE_GET_LIST_COUNT | Counts are equal                                   |                                            |
 * | 6    | For each boot reason, validate against profile  | bootReason, ut_kvp_getStringField                       | Each boot reason is present in the profile         | Checks each supported boot reason          |
 *
 */
UT_ADD_TEST(VTS_L1_BOOT, getCapabilities_pos)
{
    android::sp<com::rdk::hal::bootreason::IBootReason> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::bootreason::Capabilities capabilities;
    BOOT_GETCAPABILITIES_SUCCESS(bootHandle, capabilities);
    const std::string baseKey = "bootreason/supportedResetTypes";
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
            found = false;
            if (ut_kvp_getStringField(ut_kvp_profile_getInstance(), key.c_str(), value, sizeof(value)) == UT_KVP_STATUS_SUCCESS)
            {
                if (strcmp(value, resetTypesStr.c_str()) == 0) // Validate that the reset type string in profile matches with the reset type string from capabilities
                {
                    found = true;
                    break;
                }
            }
        }
        UT_ASSERT_TRUE(found); // Validate that each reset type from capabilities is present in profile
    }

    const std::string baseKeyBootReasons = "bootreason/supportedBootCauses";
    UT_ASSERT_EQUAL(capabilities.supportedBootCauses.size(), UT_KVP_PROFILE_GET_LIST_COUNT(baseKeyBootReasons.c_str()));
    found = false;
    for (const auto &bootReason : capabilities.supportedBootCauses)
    {
        const std::string bootReasonStr = android::internal::ToString(bootReason);
        UT_LOG_DEBUG("Supported bootReasonStr: %s\n", bootReasonStr.c_str());
        std::string key;
        for (int i = 0; i < (int)capabilities.supportedBootCauses.size(); i++)
        {
            key = baseKeyBootReasons + "/" + std::to_string(i);
            found = false;
            char value[UT_KVP_MAX_ELEMENT_SIZE] = {0};
            if (ut_kvp_getStringField(ut_kvp_profile_getInstance(), key.c_str(), value, sizeof(value)) == UT_KVP_STATUS_SUCCESS)
            {
                if (strcmp(value, bootReasonStr.c_str()) == 0) // Validate that the boot reason string in profile matches with the boot reason string from capabilities
                {
                    found = true;
                    break;
                }
            }
        }
        UT_ASSERT_TRUE(found); // Validate that each boot reason from capabilities is present in profile
    }
}

/**
 * @brief Positive test for setBootCause() function of boot interface
 *
 * This test verifies that the setBootCause() method successfully sets a valid
 * boot reason with an appropriate reason string.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 002@n
 *
 * **Test Procedure:**
 * | Step | Action                                          | Input / Test Data                                       | Expected Result                                    | Notes                                      |
 * |------|-------------------------------------------------|---------------------------------------------------------|----------------------------------------------------|-----------------------------------------|
 * | 1    | Obtain boot handle                              | gBOOTHandle                                             | bootHandle is not nullptr                          |                                            |
 * | 2    | Set boot reason to ERROR_UNKNOWN                | bootReasonToSet = BootReason::ERROR_UNKNOWN             | Boot reason enum assigned                          |                                            |
 * | 3    | Convert boot reason to string                   | toString(bootReasonToSet)                               | reasonString populated                             |                                            |
 * | 4    | Call BOOT_SETBOOTCAUSE_SUCCESS                 | bootHandle, bootReasonToSet, reasonString               | Status::ok() returned                              | Boot reason successfully set               |
 *
 */
UT_ADD_TEST(VTS_L1_BOOT, setBootCause_pos)
{
    android::sp<com::rdk::hal::bootreason::IBootReason> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::bootreason::BootCause bootReasonToSet = com::rdk::hal::bootreason::BootCause::ERROR_UNKNOWN;
    std::string reasonString = com::rdk::hal::bootreason::toString(bootReasonToSet);
    BOOT_SETBOOTCAUSE_SUCCESS(bootHandle, bootReasonToSet, reasonString);
}

/**
 * @brief Negative test for setBootCause() function of boot interface
 *
 * This test verifies that the setBootCause() method correctly rejects invalid
 * inputs including unsupported boot reason enum values and empty reason strings.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 003@n
 *
 * **Test Procedure:**
 * | Step | Action                                          | Input / Test Data                                       | Expected Result                                    | Notes                                      |
 * |------|-------------------------------------------------|---------------------------------------------------------|----------------------------------------------------|-----------------------------------------|
 * | 1    | Obtain boot handle                              | gBOOTHandle                                             | bootHandle is not nullptr                          |                                            |
 * | 2    | Set invalid boot reason                         | invalidBootReason = -999                                | Invalid enum value assigned                        |                                            |
 * | 3    | Convert invalid boot reason to string           | toString(invalidBootReason)                             | reasonString populated                             |                                            |
 * | 4    | Call BOOT_SETBOOTCAUSE_FAILURE                 | bootHandle, invalidBootReason, reasonString             | EX_ILLEGAL_ARGUMENT returned                       | Unsupported boot reason rejected           |
 * | 5    | Call BOOT_SETBOOTCAUSE_FAILURE with empty string | bootHandle, ERROR_UNKNOWN, ""                           | EX_ILLEGAL_ARGUMENT returned                       | Empty reason string rejected               |
 * | 6    | Call BOOT_SETBOOTCAUSE_FAILURE with >64 byte string | bootHandle, ERROR_UNKNOWN, 65-char string           | EX_ILLEGAL_ARGUMENT returned                       | causeString exceeding 64 bytes rejected    |
 *
 */
UT_ADD_TEST(VTS_L1_BOOT, setBootCause_neg)
{
    android::sp<com::rdk::hal::bootreason::IBootReason> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);

    // Using an unsupported boot reason to test negative case
    com::rdk::hal::bootreason::BootCause invalidBootReason = static_cast<com::rdk::hal::bootreason::BootCause>(INVALID_ENUM_TEST_VALUE);
    std::string reasonString = com::rdk::hal::bootreason::toString(invalidBootReason);
    BOOT_SETBOOTCAUSE_FAILURE(bootHandle, invalidBootReason, reasonString.c_str(), ::android::binder::Status::EX_ILLEGAL_ARGUMENT);

    // Using empty reason string to test negative case
    BOOT_SETBOOTCAUSE_FAILURE(bootHandle, com::rdk::hal::bootreason::BootCause::ERROR_UNKNOWN, "", ::android::binder::Status::EX_ILLEGAL_ARGUMENT);

    // Using a causeString exceeding the 64-byte limit to test negative case
    std::string longReasonString(65, 'a'); // 65 characters, one byte beyond the 64-byte limit
    BOOT_SETBOOTCAUSE_FAILURE(bootHandle, com::rdk::hal::bootreason::BootCause::ERROR_UNKNOWN, longReasonString.c_str(), ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

/**
 * @brief Positive test for getBootCause() function of boot interface
 *
 * This test verifies that the getBootCause() method successfully retrieves the
 * current boot reason and that the returned value is among the supported boot
 * reasons declared in capabilities.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 004@n
 *
 * **Test Procedure:**
 * | Step | Action                                          | Input / Test Data                                       | Expected Result                                    | Notes                                      |
 * |------|-------------------------------------------------|---------------------------------------------------------|----------------------------------------------------|-----------------------------------------|
 * | 1    | Obtain boot handle                              | gBOOTHandle                                             | bootHandle is not nullptr                          |                                            |
 * | 2    | Call BOOT_GETBOOTCAUSE_SUCCESS                 | bootHandle, bootReason                                  | bootReason is populated                            |                                            |
 * | 3    | Call BOOT_GETCAPABILITIES_SUCCESS               | bootHandle, capabilities                                | capabilities structure is populated                |                                            |
 * | 4    | Validate boot reason against capabilities       | bootReason, capabilities.supportedBootCauses            | bootReason matches one of supported reasons        | Ensures returned reason is valid           |
 *
 */
UT_ADD_TEST(VTS_L1_BOOT, getBootCause_pos)
{
    android::sp<com::rdk::hal::bootreason::IBootReason> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::bootreason::BootCause bootReason;
    BOOT_GETBOOTCAUSE_SUCCESS(bootHandle, bootReason);
    UT_LOG_INFO("Boot reason obtained from getBootCause(): ", std::to_string(static_cast<int>(bootReason)));

    // Validate that the returned boot reason is among the supported boot reasons in capabilities
    com::rdk::hal::bootreason::Capabilities capabilities;
    bool reasonMatched = false;
    BOOT_GETCAPABILITIES_SUCCESS(bootHandle, capabilities);
    for(const auto &reason : capabilities.supportedBootCauses)
    {
        if(bootReason == reason)
        {
            UT_PASS("Boot reason from getBootCause() is among supported boot causes in capabilities");
            reasonMatched = true;
            break;
        }
    }
    UT_ASSERT_TRUE(reasonMatched); // Validate that the returned boot reason is among supported boot causes in capabilities
}

/**
 * @brief Negative test for reboot() function of boot interface
 *
 * This test verifies that the reboot() method correctly rejects invalid inputs
 * including unsupported reset type enum values and empty reason strings.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 005@n
 *
 * **Test Procedure:**
 * | Step | Action                                          | Input / Test Data                                       | Expected Result                                    | Notes                                      |
 * |------|-------------------------------------------------|---------------------------------------------------------|----------------------------------------------------|-----------------------------------------|
 * | 1    | Obtain boot handle                              | gBOOTHandle                                             | bootHandle is not nullptr                          |                                            |
 * | 2    | Set invalid reset type                          | invalidResetType = -999                                 | Invalid enum value assigned                        |                                            |
 * | 3    | Create reason string                            | reasonString = "invalidResetType"                       | reasonString populated                             |                                            |
 * | 4    | Call BOOT_REBOOT_FAILURE                        | bootHandle, invalidResetType, reasonString              | EX_ILLEGAL_ARGUMENT returned                       | Unsupported reset type rejected            |
 * | 5    | Call BOOT_REBOOT_FAILURE with empty string      | bootHandle, FULL_SYSTEM_RESET, ""                       | EX_ILLEGAL_ARGUMENT returned                       | Empty reason string rejected               |
 * | 6    | Call BOOT_REBOOT_FAILURE with >64 byte string   | bootHandle, FULL_SYSTEM_RESET, 65-char string           | EX_ILLEGAL_ARGUMENT returned                       | causeString exceeding 64 bytes rejected    |
 *
 */
UT_ADD_TEST(VTS_L1_BOOT, reboot_neg)
{
    android::sp<com::rdk::hal::bootreason::IBootReason> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    // Using an unsupported reset type to test negative case
    com::rdk::hal::bootreason::ResetType invalidResetType = static_cast<com::rdk::hal::bootreason::ResetType>(-999);
    ::android::String16 reasonString = android::String16("invalidResetType");
    BOOT_REBOOT_FAILURE(bootHandle, invalidResetType, reasonString, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);

    // Using empty reason string to test negative case
    BOOT_REBOOT_FAILURE(bootHandle, com::rdk::hal::bootreason::ResetType::FULL_SYSTEM_RESET, android::String16(""), ::android::binder::Status::EX_ILLEGAL_ARGUMENT);

    // Using a causeString exceeding the 64-byte limit to test negative case
    std::string longReasonStr(65, 'a'); // 65 characters, one byte beyond the 64-byte limit
    BOOT_REBOOT_FAILURE(bootHandle, com::rdk::hal::bootreason::ResetType::FULL_SYSTEM_RESET, android::String16(longReasonStr.c_str()), ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

/**
 * @brief Positive test for getPowerSource() function of boot interface
 *
 * This test verifies that the getPowerSource() method successfully retrieves the
 * current power source and that the returned value matches the power source
 * defined in the configuration profile.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 006@n
 *
 * **Test Procedure:**
 * | Step | Action                                          | Input / Test Data                                       | Expected Result                                    | Notes                                      |
 * |------|-------------------------------------------------|---------------------------------------------------------|----------------------------------------------------|-----------------------------------------|
 * | 1    | Obtain boot handle                              | gBOOTHandle                                             | bootHandle is not nullptr                          |                                            |
 * | 2    | Call BOOT_GETPOWERSOURCE_SUCCESS                | bootHandle, powerSource                                 | powerSource is populated                           |                                            |
 * | 3    | Get power source from profile                   | ut_kvp_getStringField("bootreason/powerSource")          | Profile value retrieved successfully               |                                            |
 * | 4    | Validate power source against profile           | toString(powerSource), profile value                    | Power source matches profile configuration         | Ensures returned value matches config      |
 *
 */
UT_ADD_TEST(VTS_L1_BOOT, getPowerSource_pos)
{
    android::sp<com::rdk::hal::bootreason::IBootReason> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::bootreason::PowerSource powerSource;
    BOOT_GETPOWERSOURCE_SUCCESS(bootHandle, powerSource);

    // Validate that the returned power source matches the supported power sources in profile
    const std::string baseKey = "bootreason/powerSource";
    char value[UT_KVP_MAX_ELEMENT_SIZE] = {0};
    if (ut_kvp_getStringField(ut_kvp_profile_getInstance(), baseKey.c_str(), value, sizeof(value)) == UT_KVP_STATUS_SUCCESS)
    {
        UT_ASSERT_STRING_EQUAL(toString(powerSource).c_str(),value);
    }
}





/**
* @} // End of BOOT_HALTEST_CLASS
* @} // End of BOOT_HALTEST
* @} // End of BOOT
* @} // End of HPK
*/
/* End of vts_boot_l1_tests.cpp */
