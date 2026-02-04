from controller import CommsController
from serial import Serial
from comms.messages import *
import traceback

class MotorInterface(CommsController):
    # def __init__(self, port='COM4', baudrate=115200):
    #     CommsController().__init__(port, baudrate)
    initialized = False
    
    def move_by(self, deg: int = 0) -> int:
        try:
            p = MoveBy(int(deg)).pack()
            self.outbound.put(p)
        except Exception as e:
            traceback.print_exc()
            return 1 # Error
        return 0 # No error

    def get_position(self):
        try:
            p = Position(int(0)).pack()
            self.outbound.put(p)
        except Exception as e:
            traceback.print_exc()
            return 1 # Error
        return 0 # No error

    def estop(self):
        try:
            p = EStop(int(0)).pack()
            self.outbound.put(p)
        except Exception as e:
            traceback.print_exc()
            return 1 # Error
        return 0 # No error

    # Set the enable state of the motor (this can both enable AND disable)
    def motor_enable(self, enable: int=0):
        try:
            p = MotorEnable(int(enable)).pack()
            self.outbound.put(p)
        except Exception as e:
            traceback.print_exc()
            return 1 # Error
        return 0 # No error