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
# from messages import Test_Outbound

ard = Serial('COM4', baudrate=115200)
ard.bytesize=EIGHTBITS
ard.parity=PARITY_NONE
ard.stopbits=STOPBITS_ONE
# ard.set_buffer_size(256, 256)

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

read_thread = Thread(target = read_loop, args=[Packet, ard], daemon=True)

def main():
    spec = (Int32, Float)
    # packet = Packet(100, serialize(spec, [89, 9.909]))
    # packet = Test_Outbound(float(89), float(9.09)).pack()
    
    packet = MotorEnable(1).pack()
    packet.write_to(ard)
    # print(f'I printed: {packet.to_bytes()}')
    packet = MoveBy(int(151)).pack()
    packet.write_to(ard)
    # print(f'I printed: {packet.to_bytes()}')
    packet = Position(10).pack()
    packet.write_to(ard)
    # print(f'I printed: {packet.to_bytes()}')
    sleep(1)

if __name__ == "__main__":
    read_thread.start()
    while True:
        main()