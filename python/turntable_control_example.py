from motor_interface import MotorInterface
import traceback
from messages import *
from time import sleep

def print_msg(p: Packet, motor_interface: MotorInterface):
    if p.id() == Position.id():
        print(Position(p.data()))
    if p.id() == Ack.id():
        print(Ack(p.data()))
    if p.id() == IncomingMessageLengthError.id():
        print(IncomingMessageLengthError(p.data()))
    if p.id() == EStop.id():
        print(EStop(p.data()))
    if p.id() == MotorEnable.id():
        print(MotorEnable(p.data()))
    if p.id() == Initialized.id():
        print(Initialized(p.data()))
        motor_interface.initialized = True


def main():
    m = MotorInterface(port='COM4', baudrate=115200)
    
    # Wait for initialized message
    while(m.initialized == False):
        try:
            p = m.inbound.get()
            print_msg(p, m)
        except Exception as e:
            traceback.print_exc()

    m.motor_enable(1)
    m.move_by(-1011)
    while(True):
        m.get_position()
        try:
            while(m.inbound.empty() == False):
                p = m.inbound.get_nowait()
                print_msg(p, m)
        except Exception as e:
            traceback.print_exc()
        sleep(1)

if __name__ == "__main__":
    main()