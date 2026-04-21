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

from abc import ABC, abstractmethod
from raft.framework.core.logModule import logModule

class DutRebootInterface(ABC):
    """
    Abstract base class for DUT reboot controllers. 
    Defines the interface for triggering device reboots with specified boot reasons.
    """

    def __init__(self, logger: logModule):
        """
        Initialize the DUT reboot controller.
        
        Args:
            logger (logModule): Logger module instance for logging operations.
        """
        self._log = logger

    @abstractmethod
    def triggerReboot(self, boot_reason: str) -> bool:
        """
        Trigger a device reboot with the specified boot reason.
        
        Args:
            boot_reason (str): The boot reason to set (e.g., "WATCHDOG", "MAINTENANCE_REBOOT", etc.)
        
        Returns:
            bool: True if reboot triggered successfully, False otherwise
        """
        pass
