from dataclasses import dataclass, field
from enum import Enum
from typing import Optional, Tuple

import socket

# Host and port
DEFAULT_HOST = "192.168.0.1"
DEFAULT_PORT = 9876

class Client:
    def __init__(self, host: Optional[str] = None, port: Optional[int] = None):
        self.host = host or  DEFAULT_HOST
        self.port = port or DEFAULT_PORT

        self._socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def _send_data(self, data: bytes):
        self._socket.sendto(data, (self.host, self.port))

    def _send_to_ch(self, ch: int, value: int):
        if ch < 0 or ch > 3:
            raise ValueError(f"Invalid channel: {ch}")

        data = f"ch{ch} {value}"
        self._send_data(data.encode("utf-8"))

class Command(Enum):
    L          = 1250
    R          = 1900
    # Light
    LIGHTOFF   = 1200
    CABINLIGHT = 1350
    HEADLIGHT  = 1650
    # Engine
    ENGINE_OFF = 1800
    ENGINE_ON  = 1950
    # Misc
    HORN       = 1050
    EPA        = 1450

NEUTRAL_VALUE = 1500

@dataclass(slots=True)
class CarConfig:
    # Steering
    steering_auto_return: bool =  True
    steering_reverse: bool = False
    steering_limits: Tuple[int, ...] = (200, 300, 500)
    steering_limits_idx: int = field(init=False)
    # Throttle
    throttle_auto_return: bool = True
    throttle_reverse: bool = False
    throttle_limits: Tuple[int, ...] = (150, 300, 500)
    throttle_limits_idx: int = field(init=False)

    def __post_init__(self):
        # Steering and throttle limit index
        self.steering_limits_idx = len(self.steering_limits) // 2
        self.throttle_limits_idx = -1

class Car:
    def __init__(self, client: Client, config: Optional[CarConfig] = None):
        self.client = client
        self.config = config or CarConfig()

    def L(self):
        self.client._send_to_ch(0, Command.L.value)

    def R(self):
        self.client._send_to_ch(0, Command.R.value)

    # Light
    def lightoff(self):
        self.client._send_to_ch(0, Command.LIGHTOFF.value)

    def cabinlight(self):
        self.client._send_to_ch(0, Command.CABINLIGHT.value)

    def headlight(self):
        self.client._send_to_ch(0, Command.HEADLIGHT.value)

    # Engine
    def engine_on(self):
        self.client._send_to_ch(0, Command.ENGINE_ON.value)

    def engine_off(self):
        self.client._send_to_ch(0, Command.ENGINE_OFF.value)

    # Misc
    def horn(self):
        self.client._send_to_ch(0, Command.HORN.value)

    def epa(self):
        self.client._send_to_ch(0, Command.EPA.value)

    # Trimmer
    def trim(self, value: int):
        trim_value = max(-500, min(value, 500)) + NEUTRAL_VALUE
        self.client._send_to_ch(1, trim_value)

    # Steering
    def steer(self, value: int):
        steering_value = max(-self.config.steering_limits[self.config.steering_limits_idx], min(value, self.config.steering_limits[self.config.steering_limits_idx]))
        if self.config.steering_reverse:
            steering_value = -steering_value

        steering_value += NEUTRAL_VALUE
        self.client._send_to_ch(2, steering_value)

    # Throttle
    def throttle(self, value: int):
        throttle_value = max(-self.config.throttle_limits[self.config.throttle_limits_idx], min(value, self.config.throttle_limits[self.config.throttle_limits_idx]))
        if self.config.throttle_reverse:
            throttle_value = -throttle_value

        throttle_value += NEUTRAL_VALUE
        self.client._send_to_ch(3, throttle_value)
