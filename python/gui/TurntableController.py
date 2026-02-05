from TurntableControllerBase import Ui_TurntableControllerBase
from PyQt6 import QtWidgets
from PyQt6.QtCore import QTimer
from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QMainWindow
from PyQt6.QtGui import QPalette, QColor
import traceback
import sys
from motor_interface import MotorInterface
from threading import Thread

from messages import MotorEnable, Position, MoveBy
from packet import Packet

class Ui_TurntableControllerFull(Ui_TurntableControllerBase):
    def setup(self):
        # Initialize Motor interface

        try:
            self.tryConnect()    
        except:
            traceback.print_exc()
        self.incoming_message_timer = QTimer(self)
        self.incoming_message_timer.setInterval(10)
        self.incoming_message_timer.timeout.connect(self.incoming_message_handle)

        self.request_position_timer = QTimer(self)
        self.request_position_timer.setInterval(1000)
        self.request_position_timer.timeout.connect(self.request_position)

        self.tryConnect_button.clicked.connect(self.tryConnect)
        self.enableMotor_button.clicked.connect(self.enableMotor)
        self.disableMotor_button.clicked.connect(self.disableMotor)
        self.getPosition_button.clicked.connect(self.getPosition)
        self.moveMinusOne_button.clicked.connect(self.moveMinusOne)
        self.movePlusOne_button.clicked.connect(self.movePlusOne)
        self.degIndicator.setNumDigits(5)
        self.sendCommand_button.clicked.connect(self.sendCommand)

        self.incoming_message_timer.start()
        self.request_position_timer.start()
    
    def request_position(self):
        self.motor_interface.outbound.put(Position(0).pack())

    def sendCommand(self):
        # Get the spinbox number
        val = self.moveAmount_spinBox.value()
        msg = MoveBy(val)
        self.motor_interface.outbound.put(msg.pack())

    def movePlusOne(self):
        self.motor_interface.outbound.put(MoveBy(int(1)).pack())

    def moveMinusOne(self):
        self.motor_interface.outbound.put(MoveBy(int(-1)).pack())
        
    def enableMotor(self):
        self.motor_interface.outbound.put(MotorEnable(1).pack())

    def disableMotor(self):
        self.motor_interface.outbound.put(MotorEnable(0).pack())

    def getPosition(self):
        self.motor_interface.outbound.put(Position(0).pack())

    def incoming_message_handle(self):
        try:
            if(self.motor_interface is not None):
                if(not self.motor_interface.inbound.empty()):
                    p = self.motor_interface.inbound.get_nowait()
                    self.handle_message(p)
                    self.incomingMessages_plainTxtEdit.appendPlainText(p.__repr__())
        except:
            traceback.print_exc()
    
    def handle_message(self, p: Packet):
        if p.id() == Position.id():
            msg = Position.from_pack(p)
            self.degIndicator.display(msg.data/100.0)
            self.deg100Indicator.display(msg.data)

    def tryConnect(self):
        try:
            # Get port name
            port_name = self.portName_lineEdit.text()
            self.motor_interface = MotorInterface(port_name, 115200)
            self.connectionStatus_label.setText("Connected!")
        except:
            self.connectionStatus_label.setText("Not Connected!")
            traceback.print_exc()
        


class TurntableControllerGUI(QMainWindow, Ui_TurntableControllerFull):
    def __init__(self, *args, obj=None, **kwargs):
        super(TurntableControllerGUI, self).__init__(*args, **kwargs)
        self.setupUi(self)
        self.setup()
        pass


def main(*args):
    try:
        app = QtWidgets.QApplication(sys.argv)
    except:
        traceback.print_exception()
    try:
        gui = TurntableControllerGUI()
        gui.show()
    except:
        traceback.print_exc()
    ret = app.exec()
    sys.exit(ret)
    
if __name__ == "__main__":
    main()