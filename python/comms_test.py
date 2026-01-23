from serial import Serial
from time import sleep
import base64
import numpy as np
from numpy import int32
from ctypes import c_int32, c_uint32

class Int32:
	@staticmethod
	def insert(i):
		return i.to_bytes(4, byteorder='big')

	@staticmethod
	def extract(b):
		return int.from_bytes(b[:4], byteorder='big'), b[4:]


ard = Serial('COM7', baudrate=115200)
ard.set_buffer_size(8, 8)

def main():
    b64bytes = base64.standard_b64encode(int(89).to_bytes(4, byteorder='big'))
    ard.write(bytearray([b'\x01', b64bytes, b'\x02', b'0.0', b'\x03', b'\x04']))
    # ard.write(b"asdf")
    print(ard.read(ard.in_waiting))
    sleep(1)

if __name__ == "__main__":
    
    while True:
        main()