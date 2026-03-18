from TurntableControllerBase import Ui_TurntableControllerBase
from PyQt6 import QtWidgets
from PyQt6.QtCore import QTimer, QThreadPool
from PyQt6.QtCore import Qt, QObject
from PyQt6.QtCore import pyqtSignal, pyqtSlot
from PyQt6.QtWidgets import QMainWindow
from PyQt6.QtGui import QPalette, QColor
import traceback
import sys
from motor_interface import MotorInterface
from threading import Thread

from messages import MotorEnable, Position, MoveBy
from packet import Packet
from controller import Worker

class Ui_TurntableControllerFull(Ui_TurntableControllerBase):
    incoming_message_signal: pyqtSignal
    incoming_message_signal = pyqtSignal(str)
    update_position_signal = pyqtSignal(int)

    def setup(self):
        
        self.threadPool = QThreadPool()

        self.incoming_message_signal.connect(self.setPlainText)
        self.update_position_signal.connect(self.update_position_display)

        self.incoming_message_worker = Worker()
        self.incoming_message_worker.run = self.incoming_message_handle

        self.request_position_timer = QTimer(self)
        self.request_position_timer.setInterval(500)
        self.request_position_timer.timeout.connect(self.request_position)

        self.tryConnect_button.clicked.connect(self.tryConnect)
        self.enableMotor_button.clicked.connect(self.enableMotor)
        self.disableMotor_button.clicked.connect(self.disableMotor)
        self.getPosition_button.clicked.connect(self.getPosition)
        self.moveMinusOne_button.clicked.connect(self.moveMinusOne)
        self.movePlusOne_button.clicked.connect(self.movePlusOne)
        self.degIndicator.setNumDigits(5)
        self.sendCommand_button.clicked.connect(self.sendCommand)
        self.closeConnection_button.clicked.connect(self.closeConnection)
        
        # Initialize Motor interface
        self.motor_interface = None
        try:
            self.tryConnect()    
        except:
            traceback.print_exc()

    @pyqtSlot(int)
    def update_position_display(self, val: int):
        self.deg100Indicator.display(val)
        self.degIndicator.display(val/100.0)

    @pyqtSlot(str)
    def setPlainText(self, val: str):
        self.incomingMessages_plainTxtEdit.setPlainText(val)

    def closeConnection(self):
        self.motor_interface.ser.close()

    def request_position(self):
        self.motor_interface.get_position()

    def sendCommand(self):
        # Get the spinbox number
        val = self.moveAmount_spinBox.value()
        self.motor_interface.move_by(val)

    def movePlusOne(self):
        self.motor_interface.move_by(1)

    def moveMinusOne(self):
        self.motor_interface.move_by(-1)
        
    def enableMotor(self):
        self.motor_interface.outbound.put(MotorEnable(1).pack())

    def disableMotor(self):
        self.motor_interface.outbound.put(MotorEnable(0).pack())

    def getPosition(self):
        self.motor_interface.outbound.put(Position(0).pack())

    def incoming_message_handle(self):
        while True:
            try:
                if(self.motor_interface != None):
                    if(not self.motor_interface.inbound.empty()):
                        p = self.motor_interface.inbound.get_nowait()
                        self.handle_message(p)
                        # self.incoming_message_signal.emit(p.__repr__())
            except:
                traceback.print_exc()

    def handle_message(self, p: Packet):
        if p.id() == Position.id():
            print("Received Position msg")
            msg = Position.from_pack(p)
            self.update_position_signal.emit(msg.data)
            # self.degIndicator.display(msg.data/100.0)
            # self.deg100Indicator.display(msg.data)
        else:
            print(p)

    def tryConnect(self):
        try:
            # Get port name
            port_name = self.portName_lineEdit.text()
            self.motor_interface = MotorInterface(port_name, 115200)
            # self.threadPool.start(self.motor_interface.inbound_thread)
            # self.motor_interface.outbound_thread.start()
            self.threadPool.start(self.motor_interface.outbound_thread)
            self.threadPool.start(self.incoming_message_worker)
            # self.threadPool.start(self.motor_interface.inbound_thread)
            
            # self.incoming_message_timer.start()
            # self.request_position_timer.start()
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