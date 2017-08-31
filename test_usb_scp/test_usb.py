# -*- coding: utf-8 -*-
# @Author: zealotnt
# @Date:   2017-08-30 11:48:21
# @Last Modified by:   Tran Tam
# @Last Modified time: 2017-08-30 17:23:59

from styl_auto_on_off import StylAutoOnOff
import time
import os

autoOnOffPort = "/dev/ttyACM1"
maximPort = "/dev/ttyACM0"

def test_LoadFirmware():
	onOffMachine = StylAutoOnOff(autoOnOffPort)

	onOffMachine.TurnOff()
	time.sleep(1)

	count = 1
	while True:
		onOffMachine.TurnOn()
		time.sleep(1)
		ret = os.system("bash ../orcanfc-updater.sh -p %s -f /home/zealot/tmp/orca_usb/secondBootloadersigned.tar" % maximPort)
		if ret != 0:
			print ("Update maxim firmware fail")
			return None
		print ("Success %d times" % count)
		onOffMachine.TurnOff()
		time.sleep(1)
		count += 1

test_LoadFirmware()
