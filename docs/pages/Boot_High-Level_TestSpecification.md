# Boot High Level Test Specification Document

## Table of Contents

- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Introduction](#introduction)
- [Module Description](#module-description)
- [L2 Test Specification](#l2-test-specification)
- [L3 Test Specification](#l3-test-specification)

## Acronyms, Terms and Abbreviations

- `HAL`    - Hardware Abstraction Layer
- `API`    - Application Programming Interface
- `AIDL`   - Android Interface Definition Language
- `L2`     - Level 2 Testing
- `L3`     - Level 3 Testing
- `NA`     - Not Applicable

## Introduction

This document provides an overview of the testing requirements for the Boot module. It outlines the scope of testing, objectives, testing levels and approaches and specific test requirements.

- Boot Interface Definitions - [BootReason HAL](https://github.com/rdkcentral/rdk-halif-aidl/tree/0.21.0/bootreason/current/com/rdk/hal/bootreason)
- Boot Feature Profile - [HFP-BootReason](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.21.0/bootreason/current/hfp-bootreason.yaml)

## Module Description

The Boot interface provides essential functionalities for managing the boot process of a device. It includes methods to retrieve boot service capabilities, determine the reason for the device boot and perform reboots.


## L2 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
|Test Set/Get Boot Cause| Verify that the `setBootCause`/`getBootCause` API sets/returns the correct cause for the device boot. | `BootCause getBootCause()`, `setBootCause`|


## L3 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
| Test Reboot | Verify reboot API behavior for following reset types:<br>• **FULL_SYSTEM_RESET** – persistent data deleted, boot cause = WARM_RESET<br>• **INVALIDATE_CURRENT_APPLICATION_IMAGE** – current image invalidated, alternate bank selected(if supported), boot cause = WARM_RESET<br>• **FORCE_DISASTER_RECOVERY** – recovery services running, boot cause = WARM_RESET<br>• **MAINTENANCE_REBOOT** – boot cause = MAINTENANCE_REBOOT<br>• **SOFTWARE_REBOOT** – boot cause = WARM_RESET. Also validate boot cause for all the scenarios | `void reboot(ResetType resetType, String reasonString)`;<br>`BootCause getBootCause()`; |
|Test getBootCause | Verify various boot causes specified like: <br>• **WATCHDOG** - triggered by disabling watchdog feed, <br>• **MAINTENANCE_REBOOT** - triggered by API call, <br>• **WARM_RESET** - triggered by API call, <br>• **COLD_BOOT** - triggered by pulling out power supply and plugging it back, <br>• **THERMAL_RESET** - **TODO** - triggered by putting device in heat chamber <br>**RISK** : may need a specialist test rig for validation  <br>•**STR_AUTH_FAILURE** - **TODO** : triggered by using specialized bootloader that can corrupt STR auth token/signature <br>**RISK** : may need a specialist test rig for validation | `BootCause getBootCause()`, `Capabilities getCapabilities()`|
| Test boot cause override behavior | Verify that the boot cause can be overridden only when the reset type is SOFTWARE_REBOOT. When `setBootCause()` is called prior to reboot: <br>• For SOFTWARE_REBOOT, confirm the boot cause is overridden after reboot. <br>• For any other reset type, confirm the boot cause is not overridden. | `void setBootCause(BootCause cause, String causeString)`, `void reboot(ResetType resetType, String reasonString)`, `BootCause getBootCause()` |
