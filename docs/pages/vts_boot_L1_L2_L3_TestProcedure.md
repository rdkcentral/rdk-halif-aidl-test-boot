# Boot L1, L2, L3 Test Procedure

## Table of Contents
- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Build the VTS](#build-the-vts)
- [Run L1 L2 Tests Without Python](#run-l1-l2-test-without-python)
- [Run Tests in Python Environment](#run-tests-in-python-environment)
  - [Update Configuration Files](#update-configuration-files)
  - [DUT Reboot Module](#dut-reboot-module)
  - [Install the Python Packages](#install-the-python-packages)
  - [Run the Tests](#run-test-cases)
- [Notes](#note)

## Acronyms, Terms and Abbreviations

- `HAL`    - Hardware Abstraction Layer
- `VTS`    - Vendor Test Suite
- `L1`     - Level 1 Testing
- `L2`     - Level 2 Testing
- `L3`     - Level 3 Testing
- `DUT`    - Device Under Test
- `RAFT`   - Rapid Automation Framework for Testing
- `YAML`   - YAML Ain't Markup Language
- `AIDL`   - Android Interface Definition Language

## Build the VTS

### Build Linux binder

The test suite depends on the Linux binder with `AIDL` support. Follow these steps to build it:

For detailed build instructions, please refer to [Linux Binder](https://github.com/rdkcentral/linux_binder_idl/tree/1.0.0)

Here are the simplest steps outlined:

```bash
git clone -b develop 'https://github.com/rdkcentral/linux_binder_idl.git'

cd linux_binder_idl

./build-linux-binder-aidl.sh 64
```

### Build VTS binaries

Clone and build the test suite:

```bash
git clone git@github.com:rdkcentral/rdk-halif-aidl-test-boot.git
```

```bash
cd rdk-halif-aidl-test-boot
```

Set the following environment variables:

- AIDL_BIN: Path to the `AIDL` compiler (e.g., /path/to/linux_binder_idl/out/aidl).

- LINUX_BINDER_OUT_DIR: Path to the Linux binder output directory (e.g., /path/to/linux_binder_idl/local).

- UT_CORE_VERSION: Specific version of the UT-Core framework (e.g., 4.7.0). Defaults to the latest tagged version.

- VCOMPONENT_BOOT_VERSION: Specific version of the Boot Virtual Component (e.g., 1.0.0). Defaults to the latest HEAD.

- RDK_HALIF_AIDL_VERSION: Specific version of the AIDL HAL Specification (e.g., 0.12.0). Defaults to the latest HEAD.

Example:

```bash
export UT_CORE_VERSION=5.0.0
export RDK_HALIF_AIDL_VERSION=0.13.1
export VCOMPONENT_BOOT_VERSION=0.1.0
export AIDL_BIN=/path/to/linux_binder_idl/out/aidl
export LINUX_BINDER_OUT_DIR=/path/to/linux_binder_idl/local/
```

By default, `UT` logs are disabled. To enable them, set the following environment variable:

```bash
export ENABLE_UT_INFO_LOGGING=1
```

Run the build script:

```bash
./build.sh Target=<Target-name>

# Supported Targets: linux, arm
```

The build.sh script:

- Clones the `rdk-halif-aidl` repository and UT-Core framework.
- Compiles the test suite.
- Generates binaries in the `build/bin` directory.
- Generates tar file `vts-boot.tar.gz` in the `build/bin` directory.

## Run L1 L2 Test Without Python

### Transfer Tar File to Target Device

Copy the `vts` binaries (`vts-boot.tar.gz`) to the target device (e.g., using `scp`):

```bash
scp -P <port> vts-boot.tar.gz root@target-device:/tmp/
```

### Extract on Target Device

Log into the target device and extract the tarball:

```bash
ssh root@target-device
mkdir /tmp/boot
tar -zxf vts-boot.tar.gz -C /tmp/boot
cd /tmp/boot
```

### Run the VTS Test on Target Device

Run the following command to execute the VTS test:

```bash
./run.sh -p profiles/hfp-boot-vcomponent.yaml
```

## Run Tests in Python Environment

### Update Configuration Files

#### Rack Configuration File

Example Rack configuration File: [vts_rack_configuration.yml](../../host/configs/vts_rack_configuration.yml)

For more details on rack configuration refer [RAFT](https://github.com/rdkcentral/python_raft/blob/1.8.0/README.md) and [example_rack_config.yml](https://github.com/rdkcentral/python_raft/blob/1.8.0/examples/configs/example_rack_config.yml)

In this file, update the configuration to define the console sessions for the `DUT` and the outbound settings:

|Console Session|Description|
|---------------|-----------|
|default|This session is used for basic operations, such as verifying the device status and retrieving the MAC address|
|ssh_hal_test|Executes the `HAL` binary for the test case|

```yaml
rackConfig:
    rack1:
        name: "rack1"
        description: "example config at my desk"
        slot1:
            # [ name: "required", description: "optional"]
            name: "slot1"
            devices:
                - dut:
                    ip: "127.0.0.1"  # IP Address of the ADA Hub
                    description: "PC"
                    platform: "vDevice"
                    consoles:
                        - default:
                            type: "ssh"
                            port: xx22
                            username: "root"
                            ip: "xx.0.0.1" #IP address
                            password: "root"
                            prompt: "~#" # Prompt string on console
                        - ssh_hal_test:
                            type: "ssh"
                            port: xx22
                            username: "root"
                            ip: "xx.0.0.1" #IP address
                            password: "root"
                            prompt: "~#" # Prompt string on console
                        - ssh_hal_test_1:
                            type: "ssh"
                            port: xx22
                            username: "root"
                            ip: "xx.0.0.1" #IP address
                            password: "root"
                            prompt: "~#" # Prompt string on console
                    outbound:
                        download_url: "tftp://tftp-server.com/rack1/slot1/"    # download location for the CPE device
                        upload_url: "sftp://server-address/home/workspace/tftp/rack1/slot1/" # upload location
                        upload_url_base_dir: "sftp://server-address/home/workspace/tftp/rack1/slot1" # alternative upload location
                        httpProxy:    # Local Proxy if required
                        workspaceDirectory: './logs/workspace'   # Local working directory
                    dutRebootController:
                        type: virtual-dut-reboot-controller
                        vendor: "vComponent"
                        address: "xx.0.0.1" # Needs to be be filled out with IP address
                        username: "" # Needs to be filled out with login username
                        password: '' # Needs to be filled out with login password
                        port: xxx
                        control_port: xxxx
```

#### Device Configuration File

Example Device configuration File: [vts_device_configuration.yml](../../host/configs/vts_device_configuration.yml)

For more details refer [RAFT](https://github.com/rdkcentral/python_raft/blob/1.7.0/README.md) and [example_device_config.yml](https://github.com/rdkcentral/python_raft/blob/1.7.0/examples/configs/example_device_config.yml)

Ensure the `platform` should match with the `DUT` platform in [Rack Configuration](#rack-configuration-file)

```yaml
deviceConfig:
    cpe1:
        platform:   "ab" # User needs to update based on their soc
        model:      "xy" # User needs to update based on their soc
        soc_vendor: "xx" # User needs to update based on their soc
        target_directory: "/tmp/"
        boot_process_name: "Vendor-Boot"
        boot_non_volatile_directory: "/tmp/"  # User needs to update based on their soc
        boot_volatile_directory: "/home/root/" # User needs to update based on their soc
        prompt: "~#" # Prompt string on console
        wait_time: 40 # Configurable timeout value to limit the total duration of individual tests
        test:
            profile: "../../profiles/hfp-boot-vcomponent.yaml" #HFP profile used to meet platform requirements
```

### DUT Reboot Module

The [host/dutReboot/](../../host/dutReboot/) module provides a pluggable, platform-aware mechanism for triggering device reboots with a specified boot reason. It abstracts the reboot mechanism so the same test code runs unchanged on both virtual and physical devices.

#### How It Works

The `bootHelperClass` constructs a `DutRebootController` during initialization:

```python
self.dutRebootController = DutRebootController(
    log=self.log,
    platform=self.platform,        # "vDevice" or real device platform string
    session=self.session,          # SSH session (vDevice only)
    control_port=control_port      # ut-controller port (default: 8080)
)
```

At test runtime, `triggerReboot(boot_reason)` is called:

| Platform | Behaviour |
|----------|-----------|
| `vDevice` | Sends a YAML message to the ut-controller, which forwards the boot reason to the running `Vendor-Boot` process |
| Real device | Prompts the user in the console to manually trigger a reboot with the specified boot reason |

The YAML message format used for vDevice:

```yaml
boot:
  command: bootReason
  params:
    reason: "<BOOT_REASON>"
```

---

### Install the Python Packages

[host](../../host/) folder contains the script files to install required packages and set up the Python environment.

- Run install.sh once to create the virtual environment
- source activate_venv.sh
- Run install.sh again to install dependencies inside the environment.

Run the following commands:

```bash
./install.sh
source activate_venv.sh
./install.sh
```

For more details refer [Python Set up](https://github.com/rdkcentral/ut-raft/wiki/Guide-for-Setting-up-the-Python-RAFT-Testing-Suite) and [Raft Setup](https://github.com/rdkcentral/python_raft/?tab=readme-ov-file#installation)

### Run Test Cases

#### L1 and L2

Once the environment is set up, execute the test cases with the following command:

```bash
python vts_boot_L1_L2_tests.py --config </PATH>/host/tests/configs/vts_rack_configuration.yml --deviceConfig </PATH>/host/tests/configs/vts_device_configuration.yml
```

#### L3

##### Device Setup

###### vDevice

For vDevice, no external hardware needs to be connected to the setup.

**UT-Controller Usage for vDevice:**

The Boot L3 tests use the UT-Controller to simulate boot scenarios on vDevice. The UT-Controller allows the test framework to trigger specific boot reasons and simulate real world reboots.

**How UT-Controller Works:**
- The test framework communicates with the vDevice via UT-Controller on a specified port (default: 9081)
- Boot reasons are sent as YAML-formatted messages to the controller
- The vDevice Boot HAL (Vendor-Boot process) receives these messages and simulates the boot behavior
- This enables automated testing of boot reason persistence and verification

**UT-Controller Message Format:**
```yaml
boot:
  command: bootReason
  params:
    reason: "<BOOT_REASON>"
```

**Supported Boot Reasons:**
- `WATCHDOG` - Watchdog timer expiration
- `MAINTENANCE_REBOOT` - Scheduled maintenance reboot
- `THERMAL_RESET` - Thermal event (overheating)
- `WARM_RESET` - Software-initiated warm reboot
- `COLD_BOOT` - Power-on from off state
- `STR_AUTH_FAILURE` - Suspend-to-RAM authentication failure
- `ERROR_UNKNOWN` - Unknown or undetermined boot reason

**Configuration:**
The UT-Controller port is specified in the rack configuration file:
```yaml
dutRebootController:
                        type: virtual-dut-reboot-controller
                        vendor: "vComponent"
                        address: "xx.0.0.1" # Needs to be be filled out with IP address
                        username: "" # Needs to be filled out with login username
                        password: '' # Needs to be filled out with login password
                        port: xxx
                        control_port: xxx
```

**Process Flow for vDevice Testing:**
1. Test framework starts the Vendor-Boot process in background
2. Test sends boot reason via UT-Controller (using `sendMessage()`)
3. Vendor-Boot process receives and processes the boot reason
4. Device simulates reboot with specified boot reason
5. Test verifies the boot reason is correctly set and retrieved

**Note:** For real devices, the UT-Controller is not used. Instead, the test prompts the user to manually reboot the device with the appropriate boot trigger.

###### Real Device

The picture below depicts the Boot L3 Test Functionality Setup.

```mermaid
graph TB
C[PC] <--> |network cable| A
A[Network Switch ] <--> |network cable| B[DUT]
```

##### Run Tests

Once the environment is set up, execute the test cases with the following command:

```bash
python <testcase>.py --config </PATH>/host/tests/configs/vts_rack_configuration.yaml --deviceConfig </PATH>/host/tests/configs/vts_device_configuration.yaml
```

### L3 Test Cases

**Test Configuration File** [vts_l3_boot.yaml](../../test_configuration/vts_l3_boot.yaml)

#### Verify Boot Reason External Cause — `vts_boot_L3_VerifyBootReasonExternalCause.py`

**Path:** [vts_boot_L3_VerifyBootReasonExternalCause.py](../../host/tests/vts_boot_L3_VerifyBootReasonExternalCause.py)

**Purpose:** 
Validates that the device correctly identifies and reports boot reasons triggered by external events. This test triggers a reboot with each boot reason from the config and verifies the device correctly reports it after coming back up.

**Test Flow:**

```mermaid
graph TD
    A[Start Test] --> B[Read Boot Reasons from Config]
    B --> C[For Each Boot Reason]
    C --> D[Reboot Device with Boot Reason]
    D --> E[Wait for Device to Boot]
    E --> F[Reinitialize Test Framework]
    F --> G[Write Expected Boot Reason to ValidateBootReason in YAML]
    G --> H[Copy YAML Config to Target Device]
    H --> I[Run C++ Test: checkBootReason]
    I --> J{Boot Reason Matches?}
    J -->|Yes| K[Log Success]
    J -->|No| L[FAIL: Boot Reason Mismatch]
    K --> M{More Boot Reasons?}
    L --> M
    M -->|Yes| C
    M -->|No| N[PASS: All Tests Complete]
```

**Test Steps:**
- Read supported boot reasons from L3 configuration file
- For each boot reason:
  - Trigger device reboot with specified boot reason
  - Wait for device to reboot and become accessible
  - Reinitialize test framework after reboot
  - Write expected boot reason to `ValidateBootReason` in `vts_l3_boot.yaml`
  - Copy updated `vts_l3_boot.yaml` to target device
  - Execute C++ test case `checkBootReason` to verify boot reason against `ValidateBootReason`
  - Validate that retrieved boot reason matches expected value
- Log results for each boot reason test
- Report overall test success/failure

---

#### Verify Reboot — `vts_boot_L3_VerifyReboot.py`

**Path:** [vts_boot_L3_VerifyReboot.py](../../host/tests/vts_boot_L3_VerifyReboot.py)

**Purpose:**
Validates device reboot functionality with different reset types. This test verifies that the device reboots correctly with various reset types and that boot reasons are set appropriately based on the reset type used.

**Test Flow:**

```mermaid
graph TD
    A[Start Test] --> B[Read Reset Types from Config]
    B --> C[For Each Reset Type]
    C --> D{Reset Type?}
    D -->|MAINTENANCE_REBOOT| E[Create Test File in Volatile Dir]
    D -->|SOFTWARE_REBOOT| E
    D -->|FULL_SYSTEM_RESET| F[Create Test File in Non-Volatile Dir]
    D -->|Other| G[No File Creation]
    E --> H[Execute C++ Test: rebootSystem]
    F --> H
    G --> H
    H --> I[Device Reboots]
    I --> J[Wait for Device Boot]
    J --> K[Verify Device Accessible via Ping]
    K --> L{Device Up?}
    L -->|No| M[FAIL: Device Not Accessible]
    L -->|Yes| N[Reinitialize Framework]
    N --> O[Write Expected Boot Reason to ValidateBootReason in YAML]
    O --> OC[Copy YAML Config to Target Device]
    OC --> P2[Execute C++ Test: checkBootReason]
    P2 --> P{Boot Reason Correct?}
    P -->|No| Q[FAIL: Wrong Boot Reason]
    P -->|Yes| R{Check File Persistence}
    R --> S{File Should Persist?}
    S -->|Yes| T{File Exists?}
    S -->|No| U{File Deleted?}
    T -->|No| V[FAIL: File Not Persisted]
    T -->|Yes| W[Log Success]
    U -->|No| X[FAIL: File Still Exists]
    U -->|Yes| W
    W --> Y{More Reset Types?}
    M --> Y
    Q --> Y
    V --> Y
    X --> Y
    Y -->|Yes| C
    Y -->|No| Z[PASS: All Tests Complete]
```

**Test Steps:**
- Read supported reset types from L3 configuration file
- For each reset type (MAINTENANCE_REBOOT, SOFTWARE_REBOOT, FULL_SYSTEM_RESET, etc.):
  - Determine expected boot reason based on reset type
  - Create test file in appropriate directory (volatile/non-volatile) if applicable
  - Execute C++ test `rebootSystem` with user-selected reset type
  - Wait for device to reboot
  - Verify device is accessible via network ping
  - Reinitialize test framework after reboot
  - Write expected boot reason to `ValidateBootReason` in `vts_l3_boot.yaml`
  - Copy updated `vts_l3_boot.yaml` to target device
  - Execute C++ test `checkBootReason` to verify boot reason against `ValidateBootReason`
  - Validate boot reason matches expected value
- Log results for each reset type test
- Report overall test success/failure

**Expected Boot Reasons per Reset Type:**

| Reset Type | File Location | File Persists? | Expected Boot Reason |
|------------|--------------|----------------|----------------------|
| MAINTENANCE_REBOOT | Volatile | Yes | MAINTENANCE_REBOOT |
| SOFTWARE_REBOOT | Volatile | Yes | WARM_RESET |
| FULL_SYSTEM_RESET | Non-Volatile | No | WARM_RESET |
| Other | None | N/A | WARM_RESET |

---

#### Verify Override Boot Reason — `vts_boot_L3_VerifyOverrideBootReason.py`

**Path:** [vts_boot_L3_VerifyOverrideBootReason.py](../../host/tests/vts_boot_L3_VerifyOverrideBootReason.py)

**Purpose:**
Validates the boot reason override functionality by testing whether a manually set boot reason persists or is ignored after a reboot, depending on the reset type used.
- **SOFTWARE_REBOOT**: override should be preserved
- **All other reset types**: override should be ignored; actual boot reason is determined by the reset type

**Test Flow:**

```mermaid
graph TD
    A[Start Test] --> B[For Each Test Case OVR-01 to OVR-05]
    B --> C[Execute C++ Test: overrideBootReason]
    C --> D[Set Boot Reason Override]
    D --> E[Execute C++ Test: rebootSystem]
    E --> F[Device Reboots with Reset Type]
    F --> G[Wait for Device Boot]
    G --> H[Verify Device Accessible]
    H --> I{Device Up?}
    I -->|No| J[FAIL: Device Not Accessible]
    I -->|Yes| K[Reinitialize Framework]
    K --> QW[Write Expected Boot Reason to ValidateBootReason in YAML]
    QW --> QC[Copy YAML Config to Target Device]
    QC --> R[Execute C++ Test: checkBootReason]
    R --> S{Boot Reason Matches Expected?}
    S -->|No| T[FAIL: Wrong Boot Reason]
    S -->|Yes| U[Log Success]
    U --> V{More Test Cases?}
    J --> V
    T --> V
    V -->|Yes| B
    V -->|No| W[PASS: All Tests Complete]
```

**Test Steps:**
- For each test case (OVR-01 to OVR-05):
  - Execute C++ test `overrideBootReason` to set boot reason override
  - Execute C++ test `rebootSystem` with specified reset type
  - Device reboots with selected reset type
  - Wait for device to boot and become accessible
  - Verify device is reachable via network ping
  - Reinitialize test framework after reboot
  - Write expected boot reason to `ValidateBootReason` in `vts_l3_boot.yaml`
  - Copy updated `vts_l3_boot.yaml` to target device
  - Execute C++ test `checkBootReason` to retrieve actual boot reason from `ValidateBootReason`
  - Validate boot reason matches expected value
- Log results for each test case
- Report overall test success/failure

**Test Cases:**

| Test ID | Boot Reason Override | Reset Type | Expected Boot Reason | Override Behavior |
|---------|---------------------|------------|---------------------|-------------------|
| OVR-01 | WATCHDOG | SOFTWARE_REBOOT | WATCHDOG | Preserved ✓ |
| OVR-02 | WATCHDOG | MAINTENANCE_REBOOT | MAINTENANCE_REBOOT | Ignored ✓ |
| OVR-03 | THERMAL_RESET | FULL_SYSTEM_RESET | WARM_RESET | Ignored ✓ |
| OVR-04 | COLD_BOOT | INVALIDATE_CURRENT_APPLICATION_IMAGE | WARM_RESET | Ignored ✓ |
| OVR-05 | STR_AUTH_FAILURE | FORCE_DISASTER_RECOVERY | WARM_RESET | Ignored ✓ |

**Expected Behavior:**
- `SOFTWARE_REBOOT`: boot reason set via `setBootReason()` is preserved and returned by `getBootReason()` after reboot
- All other reset types: override is ignored; boot reason is determined by the reset type used

---

## Note

### Test Setup on vDevice

To validate automation on vDevice, create a password for the root user using the following command:

```bash
passwd root
```

Then, update the same password in the `vts_rack_configuration.yaml` file.

Currently, the Python `paramiko` library does not support empty passwords on vDevice, so setting a password is necessary.

### Run on Windows Environment

To run the Python environment tests in a Windows environment, follow these additional steps after checking out the repository and compiling the code:

1. Install the python environment and update the rack configuration files as in the Linux environment.

2. Move the entire project folder (including `host`) to the Windows machine.

3. Open the file located at `framework/plugins/ut_raft/interactive_shell.py`.

   - Comment out or remove the following import lines to avoid errors on Windows:
     ```python
     import pty
     import fcntl
     ```

4. After these changes, run the test cases using Python on the Windows system.
