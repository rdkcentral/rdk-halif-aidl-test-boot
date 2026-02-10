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
 * @defgroup BOOT_HALTEST_CLASS BOOT HAL Tests L3 Test Cases
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
 * @file vts_boot_l3_tests.cpp
 * @brief L3 Test Cases for the BOOT HAL (Hardware Abstraction Layer) interface.
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

// Test case to validate reboot() method of boot interface
UT_ADD_TEST(VTS_L3_BOOT, reboot_system)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    com::rdk::hal::boot::ResetType uResetType;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);

    UT_LOG_INFO("Enter reset types from below options:");
    for(const auto &resetType : android::internal::enum_values<com::rdk::hal::boot::ResetType>)
    {
        UT_LOG_INFO(" %s : %d\n", com::rdk::hal::boot::toString(resetType).c_str(), resetType);
    }
    int resetTypeInt;
    std::cin >> resetTypeInt; // taking input for reset type
    uResetType = static_cast<com::rdk::hal::boot::ResetType>(resetTypeInt);
    bool validResetType = false;

    for(const auto &resetType : android::internal::enum_values<com::rdk::hal::boot::ResetType>)
    {
        if(resetType == uResetType)
        {
            UT_LOG_INFO("Selected reset type: %s\n", com::rdk::hal::boot::toString(uResetType).c_str());
            validResetType = true;
            break;
        }
    }
    UT_ASSERT_TRUE(validResetType); // Validate that the input reset type is valid
    if (!validResetType)
    {
        UT_LOG_ERROR("Invalid reset type selected: %s\n", com::rdk::hal::boot::toString(uResetType).c_str());
        return;
    }

    ::android::String16 reasonString = android::String16(com::rdk::hal::boot::toString(uResetType).c_str());
    BOOT_REBOOT_SUCCESS(bootHandle, uResetType, reasonString);
}

// Test case to validate getBootReason() method of boot interface and check if it matches with supported boot reasons
UT_ADD_TEST(VTS_L3_BOOT, checkBootReason)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::boot::BootReason bootReason;
    BOOT_GETBOOTREASON_SUCCESS(bootHandle, bootReason);
    com::rdk::hal::boot::Capabilities capabilities;
    BOOT_GETCAPABILITIES_SUCCESS(bootHandle, capabilities);
    UT_LOG_DEBUG("Boot Reason: %s", com::rdk::hal::boot::toString(bootReason).c_str());
    bool reasonMatched = false;
    for(const auto &reason : capabilities.supportedBootReasons)
    {
        if(bootReason == reason)
        {
            UT_LOG_INFO("Boot Reason matched with supported reasons: %s", com::rdk::hal::boot::toString(bootReason).c_str());
            reasonMatched = true;
            break;
        }
    }

    UT_ASSERT_TRUE(reasonMatched); // Validate that the returned boot reason is among supported boot reasons in capabilities);
}

// Test case to validate getPowerSource() method of boot interface
UT_ADD_TEST(VTS_L3_BOOT, checkPowerSource)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::boot::PowerSource powerSource;
    BOOT_GETPOWERSOURCE_SUCCESS(bootHandle, powerSource);
    UT_LOG_DEBUG("Power Source: %s", com::rdk::hal::boot::toString(powerSource).c_str());
}

// Test case to validate override of setBootReason() method of boot interface 
// and check if the set reason is correctly returned by getBootReason()
UT_ADD_TEST(VTS_L3_BOOT, overrideBootReason)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::boot::BootReason bootReasonToSet = com::rdk::hal::boot::BootReason::MAINTENANCE_REBOOT;
    std::string reasonString = "Test boot reason from overrideBootReason test case";
    BOOT_SETBOOTREASON_SUCCESS(bootHandle, bootReasonToSet, reasonString);

    UT_LOG_INFO("Boot reason set using setBootReason: %s", com::rdk::hal::boot::toString(bootReasonToSet).c_str());
    UT_LOG_INFO("Now reboot the system to check if override works as expected, select test 1 : reboot_system\n");
}

/**
* @} // End of BOOT_HALTEST_CLASS
* @} // End of BOOT_HALTEST
* @} // End of BOOT
* @} // End of HPK
*/
/* End of vts_boot_l3_tests.cpp */