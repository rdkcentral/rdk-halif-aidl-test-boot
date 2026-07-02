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
| Boot HAL | Vendor implementation of `com.rdk.hal.bootreason.IBootReason` |

## Level 2 Test Cases Overview

The following C++ test cases validate the Boot HAL L2 functionality:

### Test Suite: VTS_L2_BOOT

| Test Case | Description | HAL APIs | L2 Test Functions | Source |
|-----------|-------------|----------|-------------------|---------|
| setAndGetBootCause | Validates setting and retrieving boot causes for all supported values | `IBootReason::setBootCause()`<br>`IBootReason::getBootCause()` | `test_l2_boot_setAndGetBootCause()` | [vts_boot_l2_tests.cpp](../../src/vts_boot_l2_tests.cpp) |

---

## Test Case Details

### Test 1: setAndGetBootCause

| Title | Details |
|-------|----------|
| Function Name | `setAndGetBootCause` |
| Description | This test validates the read-write consistency of the Boot HAL's boot cause management. It ensures that boot causes can be set via `setBootCause()` and retrieved correctly via `getBootCause()`, with the retrieved value matching the set value for all supported boot causes. |
| Test Group | 02 |
| Test Case ID | 001 |
| Priority | High |

**Pre-Conditions:** 
- Boot HAL service must be running and accessible
- Device must support boot reason persistence
- Test requires write/read permissions for boot state

**Dependencies:** 
- Boot HAL Interface: `com.rdk.hal.bootreason.IBootReason`
- UT-Core Framework

**User Interaction:** 
If user chose to run the test in interactive mode, then the test case has to be selected via console.

**Test Procedure:**

| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|-----------------|-------|
| 1 | Get Boot HAL Handle | None | Boot HAL handle should be non-null (`bootHandle != nullptr`) | Verify service is accessible |
| 2 | Get BootCause Enum Values | None | Retrieve all enum values from `com.rdk.hal.bootreason.BootCause` | Includes: ERROR_UNKNOWN, WATCHDOG, MAINTENANCE_REBOOT, THERMAL_RESET, WARM_RESET, COLD_BOOT, STR_AUTH_FAILURE |
| 3 | Iterate: Set Boot Cause - ERROR_UNKNOWN | `bootCause = ERROR_UNKNOWN`<br>`causeString = "ERROR_UNKNOWN"` | `setBootCause()` returns success<br>`Status::isOk() == true`<br>`exceptionCode == EX_NONE` | Log boot cause being tested |
| 4 | Wait for Persistence | Sleep 1 second | State persistence completes | Allow HAL to persist state |
| 5 | Get Boot Cause | None | `getBootCause()` returns success<br>Returned cause = ERROR_UNKNOWN | Verify set/get consistency |
| 6 | Validate Boot Cause Match | Compare set vs retrieved | `toString(setCause) == toString(getCause)` | Assert string equality |
| 7 | Iterate: Set Boot Cause - WATCHDOG | `bootCause = WATCHDOG`<br>`causeString = "WATCHDOG"` | `setBootCause()` returns success | Repeat for next enum value |
| 8 | Wait for Persistence | Sleep 1 second | State persistence completes | |
| 9 | Get Boot Cause | None | `getBootCause()` returns WATCHDOG | |
| 10 | Validate Boot Cause Match | Compare set vs retrieved | Strings match exactly | |
| 11 | Iterate: Set Boot Cause - MAINTENANCE_REBOOT | `bootCause = MAINTENANCE_REBOOT`<br>`causeString = "MAINTENANCE_REBOOT"` | `setBootCause()` returns success | |
| 12 | Wait for Persistence | Sleep 1 second | State persistence completes | |
| 13 | Get Boot Cause | None | `getBootCause()` returns MAINTENANCE_REBOOT | |
| 14 | Validate Boot Cause Match | Compare set vs retrieved | Strings match exactly | |
| 15 | Iterate: Set Boot Cause - THERMAL_RESET | `bootCause = THERMAL_RESET`<br>`causeString = "THERMAL_RESET"` | `setBootCause()` returns success | |
| 16 | Wait for Persistence | Sleep 1 second | State persistence completes | |
| 17 | Get Boot Cause | None | `getBootCause()` returns THERMAL_RESET | |
| 18 | Validate Boot Cause Match | Compare set vs retrieved | Strings match exactly | |
| 19 | Iterate: Set Boot Cause - WARM_RESET | `bootCause = WARM_RESET`<br>`causeString = "WARM_RESET"` | `setBootCause()` returns success | |
| 20 | Wait for Persistence | Sleep 1 second | State persistence completes | |
| 21 | Get Boot Cause | None | `getBootCause()` returns WARM_RESET | |
| 22 | Validate Boot Cause Match | Compare set vs retrieved | Strings match exactly | |
| 23 | Iterate: Set Boot Cause - COLD_BOOT | `bootCause = COLD_BOOT`<br>`causeString = "COLD_BOOT"` | `setBootCause()` returns success | |
| 24 | Wait for Persistence | Sleep 1 second | State persistence completes | |
| 25 | Get Boot Cause | None | `getBootCause()` returns COLD_BOOT | |
| 26 | Validate Boot Cause Match | Compare set vs retrieved | Strings match exactly | |
| 27 | Iterate: Set Boot Cause - STR_AUTH_FAILURE | `bootCause = STR_AUTH_FAILURE`<br>`causeString = "STR_AUTH_FAILURE"` | `setBootCause()` returns success | |
| 28 | Wait for Persistence | Sleep 1 second | State persistence completes | |
| 29 | Get Boot Cause | None | `getBootCause()` returns STR_AUTH_FAILURE | |
| 30 | Validate Boot Cause Match | Compare set vs retrieved | Strings match exactly | Final validation |
| 31 | Test Complete | None | All boot causes validated successfully | Test PASS |

## References

- Boot HAL Interface Specification - [com.rdk.hal.bootreason.IBootReason AIDL Interface](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.21.0/bootreason/current/com/rdk/hal/bootreason/IBootReason.aidl)
- Boot Cause Enumeration - [BootCause.aidl](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.21.0/bootreason/current/com/rdk/hal/bootreason/BootCause.aidl)
- Reset Type Enumeration - [ResetType.aidl](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.21.0/bootreason/current/com/rdk/hal/bootreason/ResetType.aidl)
- Power Source Enumeration - [PowerSource.aidl](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.21.0/bootreason/current/com/rdk/hal/bootreason/PowerSource.aidl)
- High Level Test Specification - [Boot High Level Test Specification](Boot_High-Level_TestSpecification.md)