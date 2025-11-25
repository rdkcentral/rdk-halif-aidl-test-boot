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
UT_ADD_TEST_TO_GROUP(VTS_L3_BOOT, UT_TESTS_L3)

class VTS_L3_BOOT : public UTCore
{
public:
    VTS_L3_BOOT() : UTCore() {}

    ~VTS_L3_BOOT() override = default;

    void SetUp() override
    {
        android::ProcessState::self()->startThreadPool();
    }

    void TearDown() override
    {
    }
};

UT_ADD_TEST(VTS_L3_BOOT, reboot_system)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    com::rdk::hal::boot::ResetType uResetType;
    std::vector<com::rdk::hal::boot::ResetType> resetTypes;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);

    std::string result;
    UT_LOG_INFO("Enter reset types from below options:");
    for(const auto &resetType : android::internal::enum_values<com::rdk::hal::boot::ResetType>)
    {
        UT_LOG_INFO(" %s : %d\n", com::rdk::hal::boot::toString(resetType).c_str(), resetType);
    }
    scanf("%d", (int*)&uResetType); // taking input for reset type
    resetTypes.push_back(uResetType);
    BOOT_REBOOT_SUCCESS(bootHandle, resetTypes);
}

UT_ADD_TEST(VTS_L3_BOOT, checkBootReason)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::boot::BootReason bootReason;
    BOOT_GETBOOTREASON_SUCCESS(bootHandle, bootReason);
    com::rdk::hal::boot::Capabilities capabilities;
    BOOT_GETCAPABILITIES_SUCCESS(bootHandle, capabilities);
    UT_LOG_DEBUG("Boot Reason: %s", com::rdk::hal::boot::toString(bootReason).c_str());
    for(const auto &reason : capabilities.supportedBootReasons)
    {
        if(bootReason == reason)
        {
            UT_LOG_INFO("Boot Reason matched with supported reasons: %s", com::rdk::hal::boot::toString(bootReason).c_str());
            break;
        }
    }
}



/**
* @} // End of BOOT_HALTEST_CLASS
* @} // End of BOOT_HALTEST
* @} // End of BOOT
* @} // End of HPK
*/
/* End of vts_boot_l1_tests.cpp */