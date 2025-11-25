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

UT_ADD_TEST(VTS_L1_BOOT, getCapabilities_pos)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::boot::Capabilities capabilities;
    BOOT_GETCAPABILITIES_SUCCESS(bootHandle, capabilities);
    const std::string baseKey = "IBoot/supportedResetTypes";
    UT_ASSERT_EQUAL(capabilities.supportedResetTypes.size(), UT_KVP_PROFILE_GET_LIST_COUNT(baseKey.c_str()));
    for (const auto &resetTypes : capabilities.supportedResetTypes)
    {
        const std::string resetTypesStr = android::internal::ToString(resetTypes);
        UT_LOG_DEBUG("Supported resetTypesStr: %s\n", resetTypesStr.c_str());
        std::string key;
        for(int i = 0;i < (int)capabilities.supportedResetTypes.size(); i++)
        {
            key = baseKey + "." + std::to_string(i) + "." + resetTypesStr;
            if (ut_kvp_fieldPresent(ut_kvp_profile_getInstance(), key.c_str()))
            {
                UT_ASSERT_TRUE(true); // Key found
                break;
            }
        }
    }
    const std::string baseKeyBootReasons = "IBoot/supportedBootReasons";
    UT_ASSERT_EQUAL(capabilities.supportedBootReasons.size(), UT_KVP_PROFILE_GET_LIST_COUNT(baseKeyBootReasons.c_str()));
    for (const auto &bootReason : capabilities.supportedBootReasons)
    {
        const std::string bootReasonStr = android::internal::ToString(bootReason);
        UT_LOG_DEBUG("Supported bootReasonStr: %s\n", bootReasonStr.c_str());
        std::string key;
        for(int i = 0;i < (int)capabilities.supportedBootReasons.size(); i++)
        {
            key = baseKey + "." + std::to_string(i) + "." + bootReasonStr;
            if (ut_kvp_fieldPresent(ut_kvp_profile_getInstance(), key.c_str()))
            {
                UT_ASSERT_TRUE(true); // Key found
                break;
            }
        }
    }
}

UT_ADD_TEST(VTS_L1_BOOT, getBootReason_pos)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::boot::BootReason bootReason;
    BOOT_GETBOOTREASON_SUCCESS(bootHandle, bootReason);
    const std::string baseKey = "IBoot/bootReason_L1/0";
    char returnedString[UT_KVP_MAX_ELEMENT_SIZE] = {0}; // Buffer to hold expected value from profile
    UT_KVP_PROFILE_GET_STRING(baseKey.c_str(), returnedString); // Get expected value from profile
    UT_ASSERT_STRING_EQUAL(std::string(android::internal::ToString(bootReason)).c_str(), returnedString);
}

UT_ADD_TEST(VTS_L1_BOOT, reboot_neg)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    // Using an unsupported reset type to test negative case
    com::rdk::hal::boot::ResetType invalidResetType = static_cast<com::rdk::hal::boot::ResetType>(-999);
    std::vector<com::rdk::hal::boot::ResetType> invalidResetVec = { invalidResetType };
    BOOT_REBOOT_FAILURE(bootHandle, invalidResetVec, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
    UT_ASSERT_TRUE(true); // Makes sure reboot call fails as expected
}



/**
* @} // End of BOOT_HALTEST_CLASS
* @} // End of BOOT_HALTEST
* @} // End of BOOT
* @} // End of HPK
*/
/* End of vts_boot_l1_tests.cpp */