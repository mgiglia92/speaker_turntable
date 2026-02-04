# /**
#  * Copyright (c) 2023  Catherine Van West <catherine.vanwest@cooper.edu>
#  * SPDX-License-Identifier: GPL-3.0-or-later
#  */

from comms.packet import Packet
from serialize import *

class Message:
    def __init__(self, data):
        self.data_ = data

    @staticmethod
    def id():
        return 0
    
    def pack(self):
        return Packet(0, serialize((Int32,), [self.data_]))

    def __repr__(self):
        return f'Message(id={self.id()}, data={deserialize((Int32,), self.data_)[0]})'

class Initialized(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id():
        return 10
    
    def pack(self):
             return Packet(10, serialize((Int32, ), [self.data_]))

class MoveBy(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id():
        return 7

    def pack(self):
        return Packet(7, serialize((Int32,), [self.data_]))
    
class Position(Message):
    def __init__(self, data: int=0):
        self.data_ = data
    
    @staticmethod
    def id():
        return 9

    def pack(self):
        return Packet(9, serialize((Int32,), [self.data_]))

class IncomingMessageLengthError(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id():
        return 1
    
    def pack(self):
        return Packet(1, serialize((Int32,), [self.data_]))

class EStop(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id():
        return 4
    
    def pack(self):
        return Packet(4, serialize((Int32,), [self.data_]))

class MotorEnable(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id():
        return 5
    
    def pack(self):
        return Packet(5, serialize((Int32,), [self.data_]))
    
class Ack(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id():
        return 2
    
    def pack(self):
        return Packet(2, serialize((Int32,), [self.data_]))

class MotionComplete(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id():
        return 3
    
    def pack(self):
        return Packet(3, serialize((Int32,), [self.data_]))