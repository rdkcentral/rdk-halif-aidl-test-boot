#!/usr/bin/env python3
#* ******************************************************************************
#*  If not stated otherwise in this file or this component's LICENSE
#*  file the following copyright and licenses apply:
#*
#*  Copyright 2026 RDK Management
#*
#*  Licensed under the Apache License, Version 2.0 (the License);
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*  http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an AS IS BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#*
#* ******************************************************************************

import os
import sys

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(dir_path)

from raft.framework.core.logModule import logModule
from abstractDutRebootController import DutRebootInterface

class ManualDutRebootController(DutRebootInterface):
    """
    Manual DUT Reboot Controller for physical devices.
    Prompts the user to manually reboot the device with the specified boot reason.
    """

    def __init__(self, logger: logModule, platform: str = "device"):
        """
        Initialize the ManualDutRebootController for manual device reboots.
        
        Args:
            logger (logModule): Logger module instance for logging operations.
            platform (str, optional): Platform/device type name for display. Defaults to "device".
        """
        super().__init__(logger)
        self.platform = platform
        self._log.info(f"ManualDutRebootController initialized for platform: {platform}")

    def triggerReboot(self, boot_reason: str) -> bool:
        """
        Prompt user to manually reboot the device with the specified boot reason.
        
        Args:
            boot_reason (str): The boot reason to set (e.g., "WATCHDOG", "MAINTENANCE_REBOOT", etc.)
        
        Returns:
            bool: True (always returns True as we assume user follows instructions)
        """
        try:
            self._log.info(f"Manual reboot required for {self.platform}")
            self._log.info(f"Boot reason: {boot_reason}")
            input(f"Please reboot {self.platform} with the specified boot reason: {boot_reason} and press Enter to continue...")
            self._log.info("User confirmed device reboot")
            return True
        except Exception as e:
            self._log.error(f"Error during manual reboot prompt: {e}")
            return False
