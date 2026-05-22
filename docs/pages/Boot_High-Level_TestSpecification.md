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

- Boot Interface Definitions - [Boot HAL](https://github.com/rdkcentral/rdk-halif-aidl/tree/0.15.0/boot/current/com/rdk/hal/boot)
- Boot Feature Profile - [HFP-Boot](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.15.0/boot/current/hfp-boot.yaml)

## Module Description

The Boot interface provides essential functionalities for managing the boot process of a device. It includes methods to retrieve boot service capabilities, determine the reason for the device boot and perform reboots.


## L2 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
|Test Set/Get Boot Reason| Verify that the setBootReason/getBootReason API sets/returns the correct reason for the device boot. | BootReason getBootReason(), setBootReason|


## L3 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
| Test Reboot | Verify reboot API behavior for following reset types:<br>• **FULL_SYSTEM_RESET** – persistent data deleted, boot reason = WARM_RESET<br>• **INVALIDATE_CURRENT_APPLICATION_IMAGE** – current image invalidated, alternate bank selected(if supported), boot reason = WARM_RESET<br>• **FORCE_DISASTER_RECOVERY** – recovery services running, boot reason = WARM_RESET<br>• **MAINTENANCE_REBOOT** – boot reason = MAINTENANCE_REBOOT<br>• **SOFTWARE_REBOOT** – boot reason = WARM_RESET. Also validate bootreason for all the scenarios | void reboot(ResetType resetType, String reasonString);<br>BootReason getBootReason(); |
|Test getBootReason | Verify various reboot reasons specified like: <br>• **WATCHDOG** - triggered by disabling watchdog feed, <br>• **MAINTENANCE_REBOOT** - triggered by API call, <br>• **WARM_RESET** - triggered by API call, <br>• **COLD_BOOT** - triggered by pulling out power supply and plugging it back, <br>• **THERMAL_RESET** - **TODO** - triggered by putting device in heat chamber <br>**RISK** : may need a specialist test rig for validation  <br>•**STR_AUTH_FAILURE** - **TODO** : triggered by using specialized bootloader that can corrupt STR auth token/signature <br>**RISK** : may need a specialist test rig for validation | BootReason getBootReason(), Capabilities getCapabilities()|
| Test boot reason override behavior | Verify that the boot reason can be overridden only when the reset type is SOFTWARE_REBOOT. When setBootReason() is called prior to reboot: <br>• For SOFTWARE_REBOOT, confirm the boot reason is overridden after reboot. <br>• For any other reset type, confirm the boot reason is not overridden. | void setBootReason(BootReason reason, String reasonString), void reboot(ResetType resetType, String reasonString), BootReason getBootReason() |
