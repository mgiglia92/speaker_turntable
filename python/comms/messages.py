# /**
#  * Copyright (c) 2023  Catherine Van West <catherine.vanwest@cooper.edu>
#  * SPDX-License-Identifier: GPL-3.0-or-later
#  */

from comms.packet import Packet

class Message:
    def __init__(self, data):
        self.data_ = data

    @staticmethod
    def id(self):
        return 0
    
    def pack(self):
        return Packet(0, self.data_)
    

class MoveBy(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id(self):
        return 7

    def pack(self):
        return Packet(7, self.data_)
    
class Position(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id(self):
        return 9

    def pack(self):
        return Packet(9, self.data_)

class IncomingMessageLengthError(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id(self):
        return 1
    
    def pack(self):
        return Packet(1, self.data_)

class EStop(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id(self):
        return 4
    
    def pack(self):
        return Packet(4, self.data_)

class MotorEnable(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id(self):
        return 5
    
    def pack(self):
        return Packet(5, self.data_)
    
class Ack(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id(self):
        return 2
    
    def pack(self):
        return Packet(2, self.data_)

class MotionComplete(Message):
    def __init__(self, data):
        self.data_ = data
    
    @staticmethod
    def id(self):
        return 3
    
    def pack(self):
        return Packet(3, self.data_)