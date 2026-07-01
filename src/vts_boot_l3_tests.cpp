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

extern android::sp<com::rdk::hal::bootreason::IBootReason> gBOOTHandle;

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

/**
 * @brief Validate device reboot functionality with user-selected reset type.
 *
 * This test ensures that the `reboot()` API successfully initiates a device reboot
 * with the specified reset type and reason string accepted from user input.
 *
 * **Test Group ID:** Basic: 03@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**@n
 * - Boot HAL service must be running and accessible@n
 * - Device must support reboot operations@n
 * - User must have console/network access to device@n
 * - Device must be in operational state@n
 *
 * **Dependencies:**@n
 * - Boot HAL Interface: `com.rdk.hal.boot.IBoot`@n
 * - UT-Core Framework@n
 * - Profile configuration with supported reset types@n
 * - Network connectivity for post-reboot verification@n
 *
 * **User Interaction:** Yes - User must select a reset type from the available options presented in the console.@n
 *
 * **Test Procedure:**@n
 * Refer to UT specification documentation [Boot_L3_Low-Level_TestSpecification.md](../docs/pages/Boot_L3_Low-Level_TestSpecification.md)
 *
 */
UT_ADD_TEST(VTS_L3_BOOT, rebootSystem)
{
    android::sp<com::rdk::hal::bootreason::IBootReason> bootHandle = gBOOTHandle;
    com::rdk::hal::bootreason::ResetType uResetType;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);

    UT_LOG_INFO("Enter reset types from below options:");
    for(const auto &resetType : android::internal::enum_values<com::rdk::hal::bootreason::ResetType>)
    {
        UT_LOG_INFO(" %s : %d\n", com::rdk::hal::bootreason::toString(resetType).c_str(), resetType);
    }
    int resetTypeInt;
    std::cin >> resetTypeInt; // taking input for reset type

    // Check if input was successful
    if (std::cin.fail())
    {
        UT_LOG_ERROR("Invalid input: expected an integer\n");
        std::cin.clear();                                                   // Clear error flags
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
        UT_ASSERT_TRUE(false);                                              // Fail the test
        return;
    }
    uResetType = static_cast<com::rdk::hal::bootreason::ResetType>(resetTypeInt);
    bool validResetType = false;

    for(const auto &resetType : android::internal::enum_values<com::rdk::hal::bootreason::ResetType>)
    {
        if(resetType == uResetType)
        {
            UT_LOG_INFO("Selected reset type: %s\n", com::rdk::hal::bootreason::toString(uResetType).c_str());
            validResetType = true;
            break;
        }
    }
    UT_ASSERT_TRUE(validResetType); // Validate that the input reset type is valid
    if (!validResetType)
    {
        UT_LOG_ERROR("Invalid reset type selected: %s\n", com::rdk::hal::bootreason::toString(uResetType).c_str());
        return;
    }

    ::android::String16 reasonString = android::String16(com::rdk::hal::bootreason::toString(uResetType).c_str());
    BOOT_REBOOT_SUCCESS(bootHandle, uResetType, reasonString);
}

/**
 * @brief Validate boot reason retrieval and verification against the expected value in the L3 test configuration file.
 *
 * This test retrieves the current boot reason via `getBootReason()` and asserts it matches the value
 * specified in the `ValidateBootReason` field of `vts_l3_boot.yaml`, which is written dynamically
 * by the Python test framework before this test is executed. It verifies proper boot reason tracking
 * across device boots.
 *
 * **Test Group ID:** Basic: 03@n
 * **Test Case ID:** 002@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**@n
 * - Boot HAL service must be running and accessible@n
 * - Device must have booted successfully@n
 * - Device must have a valid boot reason set@n
 *
 * **Dependencies:**@n
 * - Boot HAL Interface: `com.rdk.hal.boot.IBoot`@n
 * - UT-Core Framework@n
 * - Profile configuration with supported boot reasons@n
 *
 * **User Interaction:** If user chose to run the test in interactive mode, then the test case has to be selected via console.@n
 *
 * **Test Procedure:**@n
 * Refer to UT specification documentation [Boot_L3_Low-Level_TestSpecification.md](../docs/pages/Boot_L3_Low-Level_TestSpecification.md)
 *
 */
UT_ADD_TEST(VTS_L3_BOOT, checkBootReason)
{
    android::sp<com::rdk::hal::bootreason::IBootReason> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    com::rdk::hal::bootreason::BootCause bootReason;
    BOOT_GETBOOTREASON_SUCCESS(bootHandle, bootReason);
    com::rdk::hal::bootreason::Capabilities capabilities;
    BOOT_GETCAPABILITIES_SUCCESS(bootHandle, capabilities);
    UT_LOG_DEBUG("Boot Reason: %s", com::rdk::hal::bootreason::toString(bootReason).c_str());
    ut_kvp_instance_t *kvpInstance = nullptr;
    BOOT_KVP_CREATE_INSTANCE(kvpInstance, BOOT_L3_TEST_CONFIG_FILE);
    UT_ASSERT_NOT_EQUAL(kvpInstance, nullptr);
    const std::string baseKey = "Boot/L3/ValidateBootReason";
    char value[UT_KVP_MAX_ELEMENT_SIZE] = {0};
    if (ut_kvp_getStringField(kvpInstance, baseKey.c_str(), value, sizeof(value)) == UT_KVP_STATUS_SUCCESS)
    {
        UT_ASSERT_STRING_EQUAL(toString(bootReason).c_str(), value);
    }
    BOOT_KVP_DESTROY_INSTANCE(kvpInstance);
}

/**
 * @brief Validate boot reason override functionality before reboot.
 *
 * This test allows the user to set a specific boot reason via `setBootReason()` and prepares
 * for cross-reboot validation. The override is verified by running `rebootSystem` subsequently
 * and checking the retrieved boot reason post-reboot.
 *
 * **Test Group ID:** Basic: 03@n
 * **Test Case ID:** 003@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**@n
 * - Boot HAL service must be running and accessible@n
 * - Device must support boot reason persistence@n
 * - Test requires write permissions for boot state@n
 *
 * **Dependencies:**@n
 * - Boot HAL Interface: `com.rdk.hal.boot.IBoot`@n
 * - UT-Core Framework@n
 * - Test 1 (rebootSystem) for subsequent verification@n
 * - Profile configuration with supported boot reasons@n
 *
 * **User Interaction:** Yes - User must select a boot reason from the available options presented in the console, then manually execute Test 1 (rebootSystem) for verification.@n
 *
 * **Test Procedure:**@n
 * Refer to UT specification documentation [Boot_L3_Low-Level_TestSpecification.md](../docs/pages/Boot_L3_Low-Level_TestSpecification.md)
 *
 */
UT_ADD_TEST(VTS_L3_BOOT, overrideBootReason)
{
    android::sp<com::rdk::hal::bootreason::IBootReason> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    // Set boot reason from user input
    UT_LOG_INFO("Enter boot reason to set from below options:");
    for(const auto &bootReason : android::internal::enum_values<com::rdk::hal::bootreason::BootCause>)
    {
        UT_LOG_INFO(" %s : %d\n", com::rdk::hal::bootreason::toString(bootReason).c_str(), bootReason);
    }
    int bootReasonInt;
    std::cin >> bootReasonInt; // taking input for boot reason

    // Check if input was successful
    if (std::cin.fail())
    {
        UT_LOG_ERROR("Invalid input: expected an integer\n");
        std::cin.clear();                                                   // Clear error flags
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
        UT_ASSERT_TRUE(false);                                              // Fail the test
        return;
    }

    com::rdk::hal::bootreason::BootCause uBootReason = static_cast<com::rdk::hal::bootreason::BootCause>(bootReasonInt);
    bool validBootReason = false;
    for(const auto &bootReason : android::internal::enum_values<com::rdk::hal::bootreason::BootCause>)
    {
        if(bootReason == uBootReason)
        {
            UT_LOG_INFO("Selected boot reason: %s\n", com::rdk::hal::bootreason::toString(uBootReason).c_str());
            validBootReason = true;
            break;
        }
    }
    UT_ASSERT_TRUE(validBootReason); // Validate that the input boot reason is valid
    if (!validBootReason)
    {
        UT_LOG_ERROR("Invalid boot reason selected: %s\n", com::rdk::hal::bootreason::toString(uBootReason).c_str());
        return; // Exit test if invalid boot reason is selected
    }
    std::string reasonString = com::rdk::hal::bootreason::toString(uBootReason);
    BOOT_SETBOOTREASON_SUCCESS(bootHandle, uBootReason, reasonString);

    UT_LOG_INFO("Boot reason set using setBootCause: %s", com::rdk::hal::bootreason::toString(uBootReason).c_str());
    UT_LOG_INFO("Now reboot the system to check if override works as expected, select test 1 : rebootSystem\n");
}

/**
* @} // End of BOOT_HALTEST_CLASS
* @} // End of BOOT_HALTEST
* @} // End of BOOT
* @} // End of HPK
*/
/* End of vts_boot_l3_tests.cpp */
