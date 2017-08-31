# -*- coding: utf-8 -*-
# @Author: zealotnt
# @Date:   2017-08-30 11:41:31
# @Last Modified by:   Tran Tam
# @Last Modified time: 2017-08-30 11:56:37

import serial

class StylAutoOnOff():
    def __init__(self, port):
        """
        """
        self.serial = serial.Serial(port, 115200, timeout=0)

    def TurnOn(self, num=0, callback=None):
        """
        """
        self.serial.write("relay on %d" % num)
        self.serial.write("\r\n")

    def TurnOff(self, num=0, callback=None):
        """
        """
        self.serial.write("relay off %d" % num)
        self.serial.write("\r\n")
