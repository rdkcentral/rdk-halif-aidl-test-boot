# Boot HAL L2 Low Level Test Specification and Procedure Documentation

## Table of Contents

- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Scope](#scope)
- [Level 2 Test Procedure](#level-2-test-procedure)
- [Level 2 Test Cases Overview](#level-2-test-cases-overview)
- [References](#references)

## Acronyms, Terms and Abbreviations

- `HAL`  - Hardware Abstraction Layer
- `AIDL` - Android Interface Definition Language
- `API`  - Application Programming Interface
- `L2`   - Level 2 Testing
- `VTS`  - Vendor Test Suite
- `BOOT` - Boot subsystem
- `OEM`  - Original Equipment Manufacturer
- `SOC`  - System on Chip
- `RDK`  - Reference Design Kit
- `UT`   - Unit Test

## Scope

This document describes the L2 Low Level Test Specification and Procedure Documentation for the Boot HAL (Hardware Abstraction Layer). It provides detailed test specifications for validating the Boot HAL's set and get functionality.

### Level 2 Testing Scope

Level 2 testing focuses on validating the read-write consistency and state management of Boot HAL APIs. The L2 test suite verifies:

- **State Management**: Testing the ability to set and retrieve boot reasons
- **Data Persistence**: Validating that set values persist and can be retrieved correctly
- **API Consistency**: Ensuring that retrieved values match previously set values
- **Functional Correctness**: Testing all supported boot reason enumeration values

The L2 tests interact directly with the Boot HAL implementation to verify that state changes are properly persisted and retrieved, ensuring the underlying hardware/software components correctly implement the AIDL interface specifications.

## Level 2 Test Procedure

The L2 test suite validates the Boot HAL's set and get functionality by:

1. Establishing a connection to the Boot HAL service via AIDL
2. Setting boot reason values using `setBootReason()` API
3. Retrieving boot reason values using `getBootReason()` API
4. Validating that retrieved values match the previously set values
5. Iterating through all supported BootReason enumeration values
6. Verifying state persistence and API consistency

### Test Setup

#### Hardware/Software Requirements

| Required | Description |
|----------|-------------|
| Device Under Test | Target device with Boot HAL implementation |
| Test Framework | UT-Core Framework v4.7.0 or later |
| AIDL Support | Linux Binder with AIDL support |
| Boot HAL | Vendor implementation of com.rdk.hal.boot.IBoot |

## Level 2 Test Cases Overview

The following C++ test cases validate the Boot HAL L2 functionality:

### Test Suite: VTS_L2_BOOT

| Test Case | Description | HAL APIs | L2 Test Functions | Source |
|-----------|-------------|----------|-------------------|---------|
| setAndGetBootReason | Validates setting and retrieving boot reasons for all supported values | `IBoot::setBootReason()`<br>`IBoot::getBootReason()` | `test_l2_boot_setAndGetBootReason()` | [vts_boot_l2_tests.cpp](../../src/vts_boot_l2_tests.cpp) |

---

## Test Case Details

### Test 1: setAndGetBootReason

| Title | Details |
|-------|----------|
| Function Name | `setAndGetBootReason` |
| Description | This test validates the read-write consistency of the Boot HAL's boot reason management. It ensures that boot reasons can be set via `setBootReason()` and retrieved correctly via `getBootReason()`, with the retrieved value matching the set value for all supported boot reasons. |
| Test Group | 02 |
| Test Case ID | 001 |
| Priority | High |

**Pre-Conditions:** 
- Boot HAL service must be running and accessible
- Device must support boot reason persistence
- Test requires write/read permissions for boot state

**Dependencies:** 
- Boot HAL Interface: `com.rdk.hal.boot.IBoot`
- UT-Core Framework

**User Interaction:** 
If user chose to run the test in interactive mode, then the test case has to be selected via console.

**Test Procedure:**

| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|-----------------|-------|
| 1 | Get Boot HAL Handle | None | Boot HAL handle should be non-null (`bootHandle != nullptr`) | Verify service is accessible |
| 2 | Get BootReason Enum Values | None | Retrieve all enum values from `com.rdk.hal.boot.BootReason` | Includes: ERROR_UNKNOWN, WATCHDOG, MAINTENANCE_REBOOT, THERMAL_RESET, WARM_RESET, COLD_BOOT, STR_AUTH_FAILURE |
| 3 | Iterate: Set Boot Reason - ERROR_UNKNOWN | `bootReason = ERROR_UNKNOWN`<br>`reasonString = "ERROR_UNKNOWN"` | `setBootReason()` returns success<br>`Status::isOk() == true`<br>`exceptionCode == EX_NONE` | Log boot reason being tested |
| 4 | Wait for Persistence | Sleep 1 second | State persistence completes | Allow HAL to persist state |
| 5 | Get Boot Reason | None | `getBootReason()` returns success<br>Returned reason = ERROR_UNKNOWN | Verify set/get consistency |
| 6 | Validate Boot Reason Match | Compare set vs retrieved | `toString(setReason) == toString(getReason)` | Assert string equality |
| 7 | Iterate: Set Boot Reason - WATCHDOG | `bootReason = WATCHDOG`<br>`reasonString = "WATCHDOG"` | `setBootReason()` returns success | Repeat for next enum value |
| 8 | Wait for Persistence | Sleep 1 second | State persistence completes | |
| 9 | Get Boot Reason | None | `getBootReason()` returns WATCHDOG | |
| 10 | Validate Boot Reason Match | Compare set vs retrieved | Strings match exactly | |
| 11 | Iterate: Set Boot Reason - MAINTENANCE_REBOOT | `bootReason = MAINTENANCE_REBOOT`<br>`reasonString = "MAINTENANCE_REBOOT"` | `setBootReason()` returns success | |
| 12 | Wait for Persistence | Sleep 1 second | State persistence completes | |
| 13 | Get Boot Reason | None | `getBootReason()` returns MAINTENANCE_REBOOT | |
| 14 | Validate Boot Reason Match | Compare set vs retrieved | Strings match exactly | |
| 15 | Iterate: Set Boot Reason - THERMAL_RESET | `bootReason = THERMAL_RESET`<br>`reasonString = "THERMAL_RESET"` | `setBootReason()` returns success | |
| 16 | Wait for Persistence | Sleep 1 second | State persistence completes | |
| 17 | Get Boot Reason | None | `getBootReason()` returns THERMAL_RESET | |
| 18 | Validate Boot Reason Match | Compare set vs retrieved | Strings match exactly | |
| 19 | Iterate: Set Boot Reason - WARM_RESET | `bootReason = WARM_RESET`<br>`reasonString = "WARM_RESET"` | `setBootReason()` returns success | |
| 20 | Wait for Persistence | Sleep 1 second | State persistence completes | |
| 21 | Get Boot Reason | None | `getBootReason()` returns WARM_RESET | |
| 22 | Validate Boot Reason Match | Compare set vs retrieved | Strings match exactly | |
| 23 | Iterate: Set Boot Reason - COLD_BOOT | `bootReason = COLD_BOOT`<br>`reasonString = "COLD_BOOT"` | `setBootReason()` returns success | |
| 24 | Wait for Persistence | Sleep 1 second | State persistence completes | |
| 25 | Get Boot Reason | None | `getBootReason()` returns COLD_BOOT | |
| 26 | Validate Boot Reason Match | Compare set vs retrieved | Strings match exactly | |
| 27 | Iterate: Set Boot Reason - STR_AUTH_FAILURE | `bootReason = STR_AUTH_FAILURE`<br>`reasonString = "STR_AUTH_FAILURE"` | `setBootReason()` returns success | |
| 28 | Wait for Persistence | Sleep 1 second | State persistence completes | |
| 29 | Get Boot Reason | None | `getBootReason()` returns STR_AUTH_FAILURE | |
| 30 | Validate Boot Reason Match | Compare set vs retrieved | Strings match exactly | Final validation |
| 31 | Test Complete | None | All boot reasons validated successfully | Test PASS |

## References

- Boot HAL Interface Specification - [com.rdk.hal.boot.IBoot AIDL Interface](https://github.com/rdkcentral/rdk-halif-aidl/blob/main/boot/current/com/rdk/hal/boot/IBoot.aidl)
- Boot Reason Enumeration - [BootReason.aidl](https://github.com/rdkcentral/rdk-halif-aidl/blob/main/boot/current/com/rdk/hal/boot/BootReason.aidl)
- Reset Type Enumeration - [ResetType.aidl](https://github.com/rdkcentral/rdk-halif-aidl/blob/main/boot/current/com/rdk/hal/boot/ResetType.aidl)
- Power Source Enumeration - [PowerSource.aidl](https://github.com/rdkcentral/rdk-halif-aidl/blob/main/boot/current/com/rdk/hal/boot/PowerSource.aidl)
- High Level Test Specification - [Boot High Level Test Specification](https://github.com/rdkcentral/rdk-halif-aidl/blob/main/docs/pages/boot-high-level-test-spec.md)