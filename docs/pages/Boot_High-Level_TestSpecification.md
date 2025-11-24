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

- Boot Interface Definitions - [Boot HAL](https://github.com/rdkcentral/rdk-halif-aidl/tree/main/boot/current/com/rdk/hal/boot)
- Boot Feature Profile - [HFP-Boot](https://github.com/rdkcentral/rdk-halif-aidl/blob/main/boot/current/hfp-boot.yaml)

## Module Description

The Boot interface provides essential functionalities for managing the boot process of a device. It includes methods to retrieve boot service capabilities, determine the reason for the device boot, perform reboots and shutdowns, and obtain information about the device's power source.


## L2 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
|Test Get Capabilities |Verify that the getCapabilities API returns the correct capabilities of the boot service.| Capabilities getCapabilities()|
|Test Get Boot Reason| Verify that the getBootReason API returns the correct reason for the device boot.| BootReason getBootReason()|
|Test Get Power Source | Verify that the getPowerSource API returns the correct power source of the device. | PowerSource getPowerSource() |


## L3 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
|Test Reboot | Verify that the reboot API performs a shutdown and warm reboot of the device with the specified reset types. | void reboot(in ResetType[] resetTypes), Capabilities getCapabilities(), BootReason getBootReason()|