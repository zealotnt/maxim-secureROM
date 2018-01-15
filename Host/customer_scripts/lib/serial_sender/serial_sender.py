#!/usr/bin/env python2
# -*- coding: utf-8 -*-
#
# src/serial_sender.py
#
# ----------------------------------------------------------------------------
# Copyright © 2009-2015, Maxim Integrated Products
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the Maxim Integrated Products nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY MAXIM INTEGRATED PRODUCTS ''AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL MAXIM INTEGRATED PRODUCTS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ----------------------------------------------------------------------------
#
# Created on: Dec 15, 2009
# Author: Grégory Romé <gregory.rome@maxim-ic.com>
# Author: Benjamin VINOT <benjamin.vinot@maximintegrated.com>
#
# ---- Subversion keywords (need to set the keyword property)
# $Rev:: 718           $:  Revision of last commit
# $Author:: grome      $:  Author of last commit
# $Date:: 2010-02-10 1#$:  Date of last commit
#
u""" Usage: serial_sender [options] FILENAME

Options:
   | --version                  show program's version number and exit
   | -h, --help                 show this help message and exit
   | -s SERIAL, --serial=SERIAL define the serial port to use
   | -v, --verbose              enable verbose mode
   | -l, --license              display license
   | --list-serial              display available serial ports
   | -b, --bl-emulation         emulate the bootloader

serial_send sends signed packets to the bootloader on the serial link. This
tool can be used as test tool for bootloader and validation tool. FILENAME
contains both sended and received packets. Those last ones are used for
verification.

:Summary: Bootloader SCP commands sender
:Version: 2.0

:Todo:
 - Implement support of configuration file

:Author: Grégory Romé - gregory.rome@maxim-ic.com
:Author: Benjamin VINOT - benjamin.vinot@maximintegrated.com
:Organization: Maxim Integrated Products
:Copyright: Copyright © 2009-2015, Maxim Integrated Products
:License: BSD License - http://www.opensource.org/licenses/bsd-license.php
"""

__version__ = "$Revision: 718 $"
# $Source$

# ---- IMPORTS

import os
import re
import time
import progressbar
import sys
import serial

from utils import print_ok, print_err, print_noti
from optparse import OptionParser, OptionGroup
from serial.tools import list_ports
from scan import scan
from ScpPacket import *

# ---- CONSTANTS

AUTHOR = u"Grégory Romé, Benjamin VINOT"

VERSION = "2.0.2"

PROG = "serial_sender"

COPYRIGHT = u"Copyright © 2009-2015, Maxim Integrated Products"

LICENSE = u"""Copyright © 2009-2015, Maxim Integrated Products
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Maxim Integrated Products nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY MAXIM INTEGRATED PRODUCTS ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL MAXIM INTEGRATED PRODUCTS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."""

EPILOG = """serial_send sends signed packets to the bootloader on the serial
link. This tool can be used as test tool for bootloader and validation tool.
FILENAME contains both sended and received packets. Those last ones are used for
verification"""

# ---- GLOBALS

_ERROR_MSG = [['  ' for i in range(40)] for j in range(20)]

def parse_scpcmd_file(filename, bl_scp, options):
    """
    :param cmds_file:
    :param file_dir:
    :param bl_scp:
    :param options:
    :return:
    """

    if options.verbose >= VERBOSE:
        print 'Open file: ' + filename
    cmds_file = open(filename, 'r')
    file_dir = os.path.dirname(filename)

    packet_list = []

    # Get number of packets to send
    for line in cmds_file:
        file_name = line.strip()
        s_m = re.search('(\w+[_-]*\w+)\.(\d+)\.(\w+[_-]*\w+)\.((\w+[_-]*)*)\.\w+', file_name)
        if s_m is not None:
            id = s_m.group(2)
            cmd = s_m.group(4)
            way_str = s_m.group(3)
        else:
            print_err("error: wrong filename: " + file_name)
            raise Exception()

        if way_str == 'bl':
            way = False ^ options.bl_emulation
        elif way_str == 'host':
            way = True ^ options.bl_emulation
        else:
            print_err("error: wrong filename: " + file_name)
            raise Exception()

        if cmd == "connection_request" or cmd == "connection_reply":
            packet = ConnectionPacket(file_dir, file_name, bl_scp, options, cmd, id, way)
        elif cmd == "hello_reply":
            packet = HelloReplyPacket(file_dir, file_name, bl_scp, options, cmd, id, way)
        elif cmd == "erase_mem" or cmd == "del_mem":
            packet = ErasePacket(file_dir, file_name, bl_scp, options, cmd, id, way)
        elif cmd == "dump":
            packet = DumpPacket(file_dir, file_name, bl_scp, options, cmd, id, way)
        else:
            packet = ScpPacket(file_dir, file_name, bl_scp, options, cmd, id, way)

        packet_list.append(packet)

    cmds_file.close()

    return packet_list


def process_packet(packet_list, options):
    print "TOTAL> ", len(packet_list)
    sys.stdout.flush()
    if options.verbose >= VERBOSE:
        print 'Start SCP session with %d first try (use -v for details)' % options.first_retry_nb

    # Get the connection packets
    con_req = packet_list[0]
    con_reply = packet_list[1]
    con_ack = packet_list[2]

    if options.verbose >= VERBOSE:
        print_noti('Trying to Connect. Please Reset/Repower maxim for flashing')

    if options.auto_reset_uart_dtsrts:
        #print 'Reset Board through UART'
        #resetMaxim(resetMaximNormalHigh=options.resetMaximNormalHigh, resetUART=True, serial=bl_scp)

    if options.enableMaximReset == True:
        #print "Reset Maxim through GPIO"
        #resetMaxim(resetMaximNormalHigh=options.resetMaximNormalHigh, resetGPIO=True)

    bbar = progressbar.ProgressBar(widgets=[progressbar.AnimatedMarker()], maxval=options.first_retry_nb - 1).start()
    for i in bbar((i for i in range(options.first_retry_nb))):
        try:
            con_req.process()
            con_reply.process()
            break
        except KeyboardInterrupt:
            print "Keyboard Interruption : Closing connection !"
            raise Exception()
        except Exception as inst:
            pass

    con_ack.process()

    if options.verbose >= VERBOSE:
        print '\nConnected !'

    if options.verbose <= VERBOSE:
        current = 0
        bar = progressbar.ProgressBar(maxval=(len(packet_list) - 5),
                                      widgets=[progressbar.Bar('=', '[', ']'), ' ', progressbar.Percentage()]).start()

    for packet in packet_list[3:-2]:
        try:
            packet.process()
            if options.verbose <= VERBOSE:
                current += 1
                bar.update(current)
        except Exception as insts:
            if options.warningRestrictedData:
                raise RuntimeError('RestrictedProcessing')
            raise Exception()

    if options.verbose <= VERBOSE:
        bar.finish()

    if options.verbose >= VERBOSE:
        print '\nDisconnecting...'

    decon_req = packet_list[-2]
    decon_reply = packet_list[-1]
    try:
        decon_req.process()
        decon_reply.process()
        print 'Disconnected !'
    except Exception as insts:
        raise Exception()

def resetMaxim(resetMaximNormalHigh=False, resetPower=False, resetGPIO=False, resetUART=False, serial=None):
    MAXIM_RESET_PIN = 81
    MAXIM_EXPORT_GPIO="echo %d > /sys/class/gpio/export" % MAXIM_RESET_PIN
    MAXIM_PULL_LOW= "echo low  > /sys/class/gpio/gpio%s/direction" % str(MAXIM_RESET_PIN)
    MAXIM_PULL_HIGH="echo high > /sys/class/gpio/gpio%s/direction" % str(MAXIM_RESET_PIN)

    BOARD_POWER_CONTROL_PIN = 82
    BOARD_POWER_CONTROL_EXPORT_CMD = "echo %d > /sys/class/gpio/export" % BOARD_POWER_CONTROL_PIN
    BOARD_POWER_ON_CMD  = "echo high  > /sys/class/gpio/gpio%s/direction" % str(BOARD_POWER_CONTROL_PIN)
    BOARD_POWER_OFF_CMD = "echo low > /sys/class/gpio/gpio%s/direction" % str(BOARD_POWER_CONTROL_PIN)

    # Using DTR/RTS only pull low/pull high with appropriate hardware wiring
    # In this case, we are only be able to pull the Maxim low
    # The hardware wiring is
    # + DTR to B of PNP trans
    # + RTS to E of PNP trans
    # + To pull reset pin low: DTR low, RTS high
    # + To pull reset pin high: DTR + RTS both high/low
    # (to avoid the open port case -> both line is low)
    # (port not opened -> both line is high)
    # Note:
    # DTR/RTS clear/high - serial.setRTS(False)
    # DTR/RTS set/low - serial.setRTS(True)
    if resetMaximNormalHigh == True:
        if resetGPIO == True:
            os.system(MAXIM_EXPORT_GPIO)
            os.system(MAXIM_PULL_LOW)
            os.system(MAXIM_PULL_HIGH)
        if resetUART == True:
            if serial == None:
                print_err("invalid serial value, abort resetting")
                return
            print_err("Not support pull RESET pin high")
    else:
        if resetGPIO == True:
            os.system(MAXIM_EXPORT_GPIO)
            os.system(MAXIM_PULL_HIGH)
            os.system(MAXIM_PULL_LOW)
        if resetUART == True:
            if serial == None:
                print_err("invalid serial value, abort resetting")
                return
            serial.setRTS(False)
            serial.setDTR(True)
            time.sleep(0.5)
            # Pull two pin low
            serial.setRTS(True)
            serial.setDTR(True)

    if resetPower:
        os.system(BOARD_POWER_CONTROL_EXPORT_CMD)
        os.system(BOARD_POWER_OFF_CMD)
        time.sleep(0.5)
        os.system(BOARD_POWER_ON_CMD)

def WaitMaximUsbBootloader(msec_timeout):
    import pyudev

    global maxim_device
    maxim_device = None
    def device_event(action, device):
        global maxim_device
        device_dict = dict(device)

        sys.stdout.write ("Get tty device: %s" % device_dict["DEVNAME"])
        if device_dict["ID_MODEL_ID"] != "0625" and device_dict["ID_VENDOR_ID"] != "0b6a":
            print (", it's not maxim bootloader, wait")
            return
        print (" => maxim device")
        maxim_device = device_dict["DEVNAME"]

    context = pyudev.Context()
    monitor = pyudev.Monitor.from_netlink(context)
    monitor.filter_by(subsystem='tty')

    observer = pyudev.MonitorObserver(monitor, device_event)
    observer.start()

    while msec_timeout > 0 and maxim_device is None:
        time.sleep(0.001)
        msec_timeout -= 1

    observer.stop()
    return maxim_device

# ---- MAIN
if __name__ == "__main__":
    return_code = 0
    m = re.search('Rev(ision)*\s*:\s*(\d+)', __version__)
    if m is not None:
        revision = m.group(2)
    else:
        revision = "unknown"

    usage = "usage: " + PROG + " [options] FILENAME"
    version = "%prog " + VERSION + '-rev' + revision + "\n" + COPYRIGHT
    parser = OptionParser(prog=PROG, usage=usage, version=version,
                          epilog=EPILOG)
    parser.add_option("-s", "--serial", dest="serial", type="string", help="define the serial port to use")
    parser.add_option("-v", action="count", dest="verbose", help="enable verbose mode")


    parser.add_option("-l", "--license", action="store_true", dest="license", default=False, help="display license")
    parser.add_option("--list-serial", action="store_true", dest="list_serial",
                      default=False, help="display available serial ports")


    group = OptionGroup(parser, "Timming Options")
    group.add_option("-t", "--timeout", dest="timeout", type="int",
                      default=2, help="specifies the protocol timeout (s). \
By default the timeout is 10s")

    group.add_option("-e", "--erase-timeout", dest="erase_timeout", type="int",
                      default=5, help="specifies the protocol erase mem command timeout (s). \
By default the timeout is 5s")

    parser.add_option_group(group)

    parser.add_option("-f", "--first-retry", dest="first_retry_nb", type="int",
                      default=20, help="specifies the number of retry for first packet. \
By default the number is 200")

    group = OptionGroup(parser, "Extra Options")
    group.add_option("--auto-usb-detect", action="store_true", dest="auto_usb_detect",
                      default=False, help="Auto detect serial comport using Maxim's VID/PID, and detect through udev")
    group.add_option("--usb-detect-timeout", dest="usb_detect_timeout", type="int",
                      default=5000, help="specifies the Maxim's USB detect timeout (ms). By default the timeout is 5s")
    group.add_option("--auto-reset-uart-rtsdts", action="store_true", dest="auto_reset_uart_dtsrts",
                      default=False, help="Perform a reset through UART RTS before SCP session")
    group.add_option("--auto-reset-power-control", action="store_true", dest="auto_reset_power_control",
                      default=False, help="Perform a reset through power control line of A9 board")
    group.add_option("-b", "--bl-emulation", action="store_true",
                      dest="bl_emulation", default=False,
                      help="emulate the bootloader")
    group.add_option("-m", "--mpc", action="store_true", dest="mpc", default=False,
                      help="Activate mpc standard output")

    group.add_option("-d", "--dump-file", dest="dump_filename",
                      help="write dump to FILE", metavar="FILE")

    group.add_option("-c", "--chip", dest="chip_name",
                     help="Force CHIP selection for error identification", metavar="CHIP")

    group.add_option("-r", "--resetMaxim", dest="enableMaximReset", action="store_true",
                     help="Enable Maxim Reset functionality (Only in SIRIUS imx6)", default=False)

    group.add_option("--resetMaximNormalHigh", dest="resetMaximNormalHigh", action="store_true",
                     help="Set polarity of Maxim pin to high as normal", default=False)

    group.add_option("-w", "--warning", dest="warningRestrictedData", action="store_true",
                     help="Tell SCP script that this is a sensitive data, could be fail if overwritten", default=False)

    parser.add_option_group(group)

    (options, args) = parser.parse_args()
    if options.license:
        print LICENSE
        sys.exit(0)

    if options.list_serial:
        print "Available serial ports:"
        for port_name in scan():
            print '  - ' + port_name
        sys.exit(0)

    if args.__len__() != 1:
        parser.error("argument(s) missing")
        sys.exit(-1)

    filename = args[0]

    # orca7 can control power line of orcanfc
    if options.auto_reset_power_control:
        print 'Reset Board through PowerLine'
        resetMaxim(resetPower=True)

    # if the auto_usb_detect is set, we don't use the serial port name option
    if options.serial is not None and options.auto_usb_detect == True:
        print ("Note: options.auto_usb_detect is on, ignore serial option: %s" % options.serial)
        options.serial = None

    if options.serial is None:
        if options.auto_usb_detect == True:
            serial = WaitMaximUsbBootloader(options.usb_detect_timeout)
            if serial is None:
                print_err ("Can't detect Maxim's USB bootloader, exit")
                sys.exit(-1)
        elif os.name == 'nt':
            serial = 'COM1'
        else:
            serial = '/dev/ttyS0'
    else:
        serial = options.serial

    port_name = serial

    if options.verbose >= VERBOSE:
        print 'Open serial port: ' + port_name + ' (timeout: ' + str(options.timeout) + 's)'
    bl_scp = BootloaderScp(port_name, options.timeout)

    try:
        packets_list = parse_scpcmd_file(filename, bl_scp, options)
        if sum(1 for item in iter(list_ports.grep(serial))) == 0:
            print 'Waiting for device ' + serial + ' to appears'
        while sum(1 for item in iter(list_ports.grep(serial))) == 0:
            pass
        process_packet(packets_list, options)
        print_ok("SCP session OK")
    except RuntimeError:
        print_err("Restricted Data")
        return_code = 1
    except ValueError:
        print_err("Connection Failed")
        return_code = -2
    except Exception as inst:
        print inst
        print_err("error: SCP session FAILED")
        return_code = -1
    finally:
        bl_scp.close()

    sys.exit(return_code)
