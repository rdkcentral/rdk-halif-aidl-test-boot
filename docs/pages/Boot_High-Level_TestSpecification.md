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

- Boot Interface Definitions - [Boot HAL](https://github.com/rdkcentral/rdk-halif-aidl/tree/0.12.0/boot/current/com/rdk/hal/boot)
- Boot Feature Profile - [HFP-Boot](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.12.0/boot/current/hfp-boot.yaml)

## Module Description

The Boot interface provides essential functionalities for managing the boot process of a device. It includes methods to retrieve boot service capabilities, determine the reason for the device boot and perform reboots.


## L2 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
|Test Set/Get Boot Reason| Verify that the setBootReason/getBootReason API sets/returns the correct reason for the device boot. Note: This is a placehoder and will be finalized after setBootReason() is made available| BootReason getBootReason(), setBootReason|


## L3 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
|Test Reboot | Verify that the reboot API performs a reboot of the device with the specified reset types like FULL_SYSTEM_RESET | void reboot(in ResetType[] resetTypes)|
|Test getBootReason | Verify various reboot reasons specified like WATCHDOG, SOFTWARE_RESET, THERMAL_RESET, COLD_BOOT, STR_AUTH_FAILURE | BootReason getBootReason(), Capabilities getCapabilities()|
