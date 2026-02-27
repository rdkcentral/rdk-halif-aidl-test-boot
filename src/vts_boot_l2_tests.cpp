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
 * @defgroup BOOT_HALTEST_CLASS BOOT HAL Tests L2 Test Cases
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
 * @file vts_boot_l2_tests.cpp
 * @brief L2 Test Cases for the BOOT HAL (Hardware Abstraction Layer) interface.
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
UT_ADD_TEST_TO_GROUP(VTS_L2_BOOT, UT_TESTS_L2)

class VTS_L2_BOOT : public UTCore
{
public:
    VTS_L2_BOOT() : UTCore() {}

    ~VTS_L2_BOOT() override = default;

    void SetUp() override
    {
        android::ProcessState::self()->startThreadPool();
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Validate setBootReason() and getBootReason() functionality for all supported boot reasons.
 *
 * This test verifies the correct operation of setBootReason() and getBootReason() methods
 * by iterating through all available boot reason enum values. It sets each boot reason
 * and immediately retrieves it to ensure the value persists correctly and matches the
 * originally set value. This validates the complete round-trip behavior of boot reason
 * storage and retrieval.
 *
 * **Test Group ID:** Basic: 02@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**@n
 * - Boot interface must be initialized and available@n
 * - Boot service must be running@n
 *
 * **Dependencies:**
 * - A valid gBOOTHandle must be available
 *
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * Refer to UT specification documentation [Boot_L2_Low-Level_TestSpecification.md](../docs/pages/Boot_L2_Low-Level_TestSpecification.md)
 *
 */
UT_ADD_TEST(VTS_L2_BOOT, setAndGetBootReason)
{
    android::sp<com::rdk::hal::boot::IBoot> bootHandle = gBOOTHandle;
    UT_ASSERT_NOT_EQUAL(bootHandle, nullptr);
    std::string result;

    UT_LOG_INFO("Iterating over all available boot reasons:");
    for (const auto &bootReason : android::internal::enum_values<com::rdk::hal::boot::BootReason>)
    {
        std::string reasonString = com::rdk::hal::boot::toString(bootReason);
        UT_LOG_INFO(" %s : %d\n", reasonString.c_str(), bootReason);

        BOOT_SETBOOTREASON_SUCCESS(bootHandle, bootReason, reasonString);
        sleep(1); // Sleep for a short duration to ensure the boot reason is set before we attempt to get it back

        com::rdk::hal::boot::BootReason bootReasonReturned;
        BOOT_GETBOOTREASON_SUCCESS(bootHandle, bootReasonReturned);

        // Validate that the set boot reason matches the returned boot reason
        UT_ASSERT_STRING_EQUAL(reasonString.c_str(), com::rdk::hal::boot::toString(bootReasonReturned).c_str());
    }
}

/**
* @} // End of BOOT_HALTEST_CLASS
* @} // End of BOOT_HALTEST
* @} // End of BOOT
* @} // End of HPK
*/
/* End of vts_boot_l2_tests.cpp */
