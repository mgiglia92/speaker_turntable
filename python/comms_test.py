from serial import Serial, EIGHTBITS, PARITY_NONE, STOPBITS_ONE
from time import sleep
import base64
import struct
import numpy as np
from numpy import int32
from ctypes import c_int32, c_uint32
from serialize import *
# from packet import Packet, start_tx, start_data, end_data, end_tx
from comms.messages import *
from comms.packet import *
# from packet import *
from threading import Thread
from controller import CommsController
# from messages import Test_Outbound
import traceback



def read_loop(cls: Packet, ser: Serial):
    while True:
        try:
            # ser.read_until(expected=start_tx)
            # print(cls.from_bytes(ser.read_until(expected=end_tx)).__repr__())
            print(ser.read_until(end_tx))
            # print(ser.read(16))
            # print(ser.read_until(b"bin_buffer"))
        except:
            pass


def main():
    comms = CommsController('COM4', baudrate=115200)
    # read_thread = Thread(target = read_loop, args=[Packet, comms], daemon=True)
    # read_thread.start()
    while(True):
        spec = (Int32, Float)
        # packet = Packet(100, serialize(spec, [89, 9.909]))
        # packet = Test_Outbound(float(89), float(9.09)).pack()
        
        packet = MotorEnable(1).pack()
        comms.outbound.put(packet)
        # packet.write_to(comms)
        # # print(f'I printed: {packet.to_bytes()}')
        packet = MoveBy(-151).pack()

        comms.outbound.put(packet)
        # packet.write_to(comms)
        # # print(f'I printed: {packet.to_bytes()}')
        packet = Position(10).pack()
        comms.outbound.put(packet)
        # packet.write_to(comms)
        # # print(f'I printed: {packet.to_bytes()}')
        try:
            while(comms.inbound.empty() == False):
                p = comms.inbound.get_nowait()
                print_msg(p)
        except Exception as e:
            traceback.print_exc()
        sleep(1)

def print_msg(p: Packet):
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

if __name__ == "__main__":
        main()