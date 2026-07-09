# Boot HAL L3 Low Level Test Specification and Procedure Documentation

## Table of Contents

- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Scope](#scope)
- [Level 3 Test Procedure](#level-3-test-procedure)
- [Level 3 Test Cases Overview](#level-3-test-cases-overview)
- [References](#references)

## Acronyms, Terms and Abbreviations

- `HAL`  - Hardware Abstraction Layer
- `AIDL` - Android Interface Definition Language
- `API`  - Application Programming Interface
- `L3`   - Level 3 Testing
- `VTS`  - Vendor Test Suite
- `BOOT` - Boot subsystem
- `OEM`  - Original Equipment Manufacturer
- `SOC`  - System on Chip
- `RDK`  - Reference Design Kit
- `UT`   - Unit Test

## Scope

This document describes the L3 Low Level Test Specification and Procedure Documentation for the Boot HAL (Hardware Abstraction Layer). It provides detailed test specifications for validating the Boot HAL's functionality in real-world scenarios.

### Level 3 Testing Scope

Level 3 testing focuses on end-to-end functional validation of Boot HAL APIs with actual hardware operations. The L3 test suite verifies:

- **Reboot Operations**: Testing device reboot functionality with various reset types
- **Boot Reason Verification**: Validating boot reasons after device reboots
- **Boot Reason Override**: Testing the ability to override boot reasons before reboot
- **State Persistence**: Ensuring boot-related state persists across reboots
- **Real Hardware Integration**: Testing with actual device hardware and firmware

The L3 tests require user interaction and device reboots, validating that the Boot HAL implementation correctly interacts with the underlying hardware and firmware components.

## Level 3 Test Procedure

The L3 test suite validates the Boot HAL's functionality through real device operations:

1. Establishing a connection to the Boot HAL service via AIDL
2. Accepting user input for test parameters (reset types, boot reasons)
3. Executing device reboot operations using `reboot()` API
4. Verifying boot causes using `getBootCause()` after reboot
5. Testing boot cause override functionality with `setBootCause()`
6. Validating actual device behavior matches expected outcomes
7. Verifying state persistence across reboots

### Test Setup

#### Hardware/Software Requirements

| Required | Description |
|----------|-------------|
| Device Under Test | Target device with Boot HAL implementation |
| Test Framework | UT-Core Framework v4.7.0 or later |
| AIDL Support | Linux Binder with AIDL support |
| Boot HAL | Vendor implementation of `com.rdk.hal.bootreason.IBootReason` |
| Network Access | Required for verifying device availability after reboot |
| Console Access | Required for monitoring reboot process |
| Profile Configuration | YAML profile defining supported boot capabilities |

## Level 3 Test Cases Overview

The following C++ test cases validate the Boot HAL L3 functionality:

### Test Suite: VTS_L3_BOOT

| Test Case | Description | HAL APIs | L3 Test Functions | Source |
|-----------|-------------|----------|-------------------|---------|
| rebootSystem | Validates device reboot with user-selected reset type | `IBootReason::reboot()` | `test_l3_boot_rebootSystem()` | [vts_boot_l3_tests.cpp](../../src/vts_boot_l3_tests.cpp) |
| checkBootReason | Validates boot cause retrieval and verification against the expected value in the L3 test configuration file | `IBootReason::getBootCause()`<br>`IBootReason::getCapabilities()` | `test_l3_boot_checkBootReason()` | [vts_boot_l3_tests.cpp](../../src/vts_boot_l3_tests.cpp) |
| overrideBootReason | Validates boot cause override functionality before reboot | `IBootReason::setBootCause()`<br>`IBootReason::getBootCause()` | `test_l3_boot_overrideBootReason()` | [vts_boot_l3_tests.cpp](../../src/vts_boot_l3_tests.cpp) |

---

## Test Case Details

### Test 1: rebootSystem

| Title | Details |
|-------|----------|
| Function Name | `rebootSystem` |
| Description | This test validates the device reboot functionality with user-selected reset type. It ensures that the `reboot()` API successfully initiates a device reboot with the specified reset type and reason string. |
| Test Group | 03 |
| Test Case ID | 001 |
| Priority | High |

**Pre-Conditions:** 
- Boot HAL service must be running and accessible
- Device must support reboot operations
- User must have console/network access to device
- Device must be in operational state

**Dependencies:** 
- Boot HAL Interface: `com.rdk.hal.bootreason.IBootReason`
- UT-Core Framework
- Profile configuration with supported reset types
- Network connectivity for post-reboot verification

**User Interaction:** 
Yes - User must select a reset type from the available options presented in the console.

**Test Flow Diagram:**

```mermaid
graph TD
    A[Get Boot HAL Handle] --> B{Handle Valid?}
    B -->|No| C[FAIL: Handle is NULL]
    B -->|Yes| D[Display Reset Type Options]
    D --> E[Accept User Input]
    E --> F{Valid Integer Input?}
    F -->|No| G[FAIL: Invalid Input]
    F -->|Yes| H[Convert to ResetType Enum]
    H --> I{Valid ResetType?}
    I -->|No| J[FAIL: Invalid Reset Type]
    I -->|Yes| K[Log Selected Reset Type]
    K --> L[Create Reason String]
    L --> M[Call reboot API]
    M --> N{Reboot Success?}
    N -->|No| O[FAIL: Reboot Failed]
    N -->|Yes| P[Device Reboots]
    P --> Q[PASS: Test Complete]
```

**Test Procedure:**

| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|-----------------|-------|
| 1 | Get Boot HAL Handle | None | Boot HAL handle should be non-null (`bootHandle != nullptr`) | Verify service is accessible |
| 2 | Display Reset Type Options | None | Display all ResetType enum values to user | Includes: FULL_SYSTEM_RESET, INVALIDATE_CURRENT_APPLICATION_IMAGE, FORCE_DISASTER_RECOVERY, MAINTENANCE_REBOOT, SOFTWARE_REBOOT |
| 3 | Accept User Input | User selects reset type by entering integer | Valid integer input received | User chooses reset type index |
| 4 | Validate Input | Check if input is valid integer | Input validation succeeds | Handle invalid input gracefully |
| 5 | Convert to ResetType Enum | Cast integer to ResetType | Valid ResetType enum value obtained | |
| 6 | Verify Reset Type is Supported | Check against enum values | Selected reset type matches a valid enum value | Log selected reset type |
| 7 | Create Reason String | Convert enum to string | Reason string created from reset type | e.g., "MAINTENANCE_REBOOT" |
| 8 | Call reboot() API | `resetType` + `reasonString` | `reboot()` returns success<br>`Status::isOk() == true`<br>`exceptionCode == EX_NONE` | Device begins reboot process |
| 9 | Device Reboots | None | Device successfully reboots | Test ends, device restarts |

---

### Test 2: checkBootReason

| Title | Details |
|-------|----------|
| Function Name | `checkBootReason` |
| Description | This test validates that the boot cause retrieved from the device matches the expected boot cause specified in the `ValidateBootReason` field of the L3 test configuration file (`vts_l3_boot.yaml`). The expected value is written dynamically by the Python test framework before this test is executed. It verifies proper boot cause tracking across device boots. |
| Test Group | 03 |
| Test Case ID | 002 |
| Priority | High |

**Pre-Conditions:** 
- Boot HAL service must be running and accessible
- Device must have booted successfully
- Device must have a valid boot reason set

**Dependencies:** 
- Boot HAL Interface: `com.rdk.hal.bootreason.IBootReason`
- UT-Core Framework
- Profile configuration with supported boot causes

**User Interaction:** 
If user chose to run the test in interactive mode, then the test case has to be selected via console.

**Test Flow Diagram:**

```mermaid
graph TD
    A[Get Boot HAL Handle] --> B{Handle Valid?}
    B -->|No| C[FAIL: Handle is NULL]
    B -->|Yes| D[Call getBootReason]
    D --> E{getBootCause Success?}
    E -->|No| F[FAIL: getBootCause Failed]
    E -->|Yes| G[Log Boot Cause]
    G --> H[Call getCapabilities]
    H --> I{getCapabilities Success?}
    I -->|No| J[FAIL: getCapabilities Failed]
    I -->|Yes| K[Read ValidateBootReason from YAML Config]
    K --> L{ValidateBootReason Found?}
    L -->|No| M[Skip Assertion]
    L -->|Yes| N{toString bootReason == ValidateBootReason?}
    N -->|No| O[FAIL: Boot Reason Mismatch]
    N -->|Yes| P[Log Match Result]
    P --> Q[PASS: Test Complete]
```

**Test Procedure:**

| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|-----------------|-------|
| 1 | Get Boot HAL Handle | None | Boot HAL handle should be non-null (`bootHandle != nullptr`) | Verify service is accessible |
| 2 | Get Current Boot Cause | None | `getBootCause()` returns success<br>Valid BootCause enum value returned | Retrieve current boot cause |
| 3 | Log Boot Cause | Boot cause value | Boot cause logged successfully | Display boot cause string |
| 4 | Get Device Capabilities | None | `getCapabilities()` returns success<br>Capabilities structure populated | |
| 5 | Read ValidateBootReason from YAML | `BootReason/L3/ValidateBootReason` key in `vts_l3_boot.yaml` | `ValidateBootReason` string value obtained | Value is written by the Python test framework before this test runs |
| 6 | Assert Boot Cause Matches | `toString(bootCause)` vs `ValidateBootReason` | `UT_ASSERT_STRING_EQUAL` passes | Boot cause must exactly match the expected value from YAML |
| 7 | Log Match Result | Matched boot reason | Log successful match | Display matched boot reason |
| 8 | Test Complete | None | Boot reason validated successfully | Test PASS |

---

### Test 3: overrideBootReason

| Title | Details |
|-------|----------|
| Function Name | `overrideBootReason` |
| Description | This test validates the boot cause override functionality. It allows the user to set a specific boot cause and prepares for verification after a subsequent reboot. This tests the `setBootCause()` API and prepares for cross-reboot validation. |
| Test Group | 03 |
| Test Case ID | 003 |
| Priority | High |

**Pre-Conditions:** 
- Boot HAL service must be running and accessible
- Device must support boot reason persistence
- Test requires write permissions for boot state

**Dependencies:** 
- Boot HAL Interface: `com.rdk.hal.bootreason.IBootReason`
- UT-Core Framework
- Test 1 (rebootSystem) for subsequent verification
- Profile configuration with supported boot causes

**User Interaction:** 
Yes - User must select a boot cause from the available options presented in the console, then manually execute Test 1 (rebootSystem) for verification.

**Test Flow Diagram:**

```mermaid
graph TD
    A[Get Boot HAL Handle] --> B{Handle Valid?}
    B -->|No| C[FAIL: Handle is NULL]
    B -->|Yes| D[Display Boot Cause Options]
    D --> E[Accept User Input]
    E --> F{Valid Integer Input?}
    F -->|No| G[FAIL: Invalid Input]
    F -->|Yes| H[Convert to BootCause Enum]
    H --> I{Valid BootCause?}
    I -->|No| J[FAIL: Invalid Boot Cause]
    I -->|Yes| K[Log Selected Boot Cause]
    K --> L[Create Cause String]
    L --> M[Call setBootCause]
    M --> N{setBootCause Success?}
    N -->|No| O[FAIL: setBootCause Failed]
    N -->|Yes| P[Log Override Success]
    P --> Q[Display Next Steps Message]
    Q --> R[PASS: Override Set - Reboot Required]
```

**Test Procedure:**

| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|-----------------|-------|
| 1 | Get Boot HAL Handle | None | Boot HAL handle should be non-null (`bootHandle != nullptr`) | Verify service is accessible |
| 2 | Display Boot Cause Options | None | Display all BootCause enum values to user | Includes: ERROR_UNKNOWN, WATCHDOG, MAINTENANCE_REBOOT, THERMAL_RESET, WARM_RESET, COLD_BOOT, STR_AUTH_FAILURE |
| 3 | Accept User Input | User selects boot cause by entering integer | Valid integer input received | User chooses boot cause index |
| 4 | Validate Input | Check if input is valid integer | Input validation succeeds | Handle invalid input gracefully |
| 5 | Convert to BootCause Enum | Cast integer to BootCause | Valid BootCause enum value obtained | |
| 6 | Verify Boot Cause is Supported | Check against enum values | Selected boot cause matches a valid enum value | Log selected boot cause |
| 7 | Create Cause String | Convert enum to string | Cause string created from boot cause | e.g., "MAINTENANCE_REBOOT" |
| 8 | Call setBootCause() API | `bootCause` + `causeString` | `setBootCause()` returns success<br>`Status::isOk() == true`<br>`exceptionCode == EX_NONE` | Boot cause override set |
| 9 | Log Override Success | Set boot cause | Log successful boot cause override | Display set boot cause |
| 10 | Display Next Steps | None | Instruct user to run Test 1 (rebootSystem) | User must manually verify with reboot |
| 11 | Test Complete | None | Boot cause override completed successfully | Test PASS - Requires subsequent reboot for full verification |

---

## References

- Boot HAL Interface Specification - [com.rdk.hal.bootreason.IBootReason AIDL Interface](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.21.0/bootreason/current/com/rdk/hal/bootreason/IBootReason.aidl)
- Boot Cause Enumeration - [BootCause.aidl](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.21.0/bootreason/current/com/rdk/hal/bootreason/BootCause.aidl)
- Reset Type Enumeration - [ResetType.aidl](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.21.0/bootreason/current/com/rdk/hal/bootreason/ResetType.aidl)
- Capabilities Structure - [Capabilities.aidl](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.21.0/bootreason/current/com/rdk/hal/bootreason/Capabilities.aidl)
- High Level Test Specification - [Boot High Level Test Specification](Boot_High-Level_TestSpecification.md)
