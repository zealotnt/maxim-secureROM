# -*- coding: utf-8 -*-
# @Author: zealotnt
# @Date:   2017-08-30 11:48:21
# @Last Modified by:   Tran Tam
# @Last Modified time: 2017-08-30 17:23:59

from styl_auto_on_off import StylAutoOnOff
import time
import os
import sys

if os.name == "posix":
	autoOnOffPort = "/dev/ttyACM1"
	maximPort = "/dev/ttyACM0"
	loadFirmwareCmd = ("bash ../orcanfc-updater.sh -p %s"
	" -f /home/zealot/tmp/orca_usb/secondBootloadersigned.tar" % maximPort)
elif os.name == "nt":
	autoOnOffPort = "COM24"
	maximPort = "COM31"
	loadFirmwareCmd = ("python ../Host/customer_scripts/lib/serial_sender/serial_sender.py"
	" -s %s -t 2 -v -w tempFw/packet.list" % maximPort)
else:
	print("OS not supported")
	sys.exit(-1)

def testOnOff():
	onOffMachine = StylAutoOnOff(autoOnOffPort)
	print("Turn off")
	onOffMachine.TurnOff()
	time.sleep(1)
	print("Turn on")
	onOffMachine.TurnOn()
	time.sleep(1)
	print("Turn off")
	onOffMachine.TurnOff()
	time.sleep(1)

def test_LoadFirmware():
	onOffMachine = StylAutoOnOff(autoOnOffPort)

	onOffMachine.TurnOff()
	time.sleep(1)

	count = 1
	while True:
		onOffMachine.TurnOn()
		time.sleep(2)
		ret = os.system(loadFirmwareCmd)
		if ret != 0:
			onOffMachine.TurnOff()
			print ("Update maxim firmware fail, turn off usb")
			return None
		print ("Success %d times" % count)
		onOffMachine.TurnOff()
		time.sleep(1)
		count += 1

# testOnOff()
test_LoadFirmware()
