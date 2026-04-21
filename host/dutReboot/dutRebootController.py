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

from dutRebootModules.virtualDutRebootController import VirtualDutRebootController
from dutRebootModules.manualDutRebootController import ManualDutRebootController

class DutRebootController:
    """
    High-level DUT reboot controller factory.
    Instantiates the correct controller type based on platform configuration.
    """

    def __init__(self, log, platform: str, session=None, control_port: int = 8080):
        """
        Initialize the DUT Reboot Controller with appropriate backend based on platform.
        
        Args:
            log: Logger module instance for logging operations
            platform (str): Platform type ('vDevice' for virtual, others for manual)
            session: SSH session (required for vDevice)
            control_port (int, optional): Control port for vDevice. Defaults to 8080.
        """
        self._log = log
        self.platform = platform

        if platform == "vDevice":
            if session is None:
                raise ValueError("Session is required for vDevice platform")
            self.controller = VirtualDutRebootController(
                logger=self._log,
                session=session,
                control_port=control_port
            )
            self._log.info("Initialized VirtualDutRebootController for vDevice")
        else:
            self.controller = ManualDutRebootController(
                logger=self._log,
                platform=platform
            )
            self._log.info(f"Initialized ManualDutRebootController for {platform}")

    def triggerReboot(self, boot_reason: str) -> bool:
        """
        Trigger a device reboot with the specified boot reason.
        
        Args:
            boot_reason (str): The boot reason to set (e.g., "WATCHDOG", "MAINTENANCE_REBOOT", etc.)
        
        Returns:
            bool: True if reboot triggered successfully, False otherwise
        """
        return self.controller.triggerReboot(boot_reason)
