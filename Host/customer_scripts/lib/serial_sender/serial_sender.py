#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# src/serial_sender.py
#
# ----------------------------------------------------------------------------
# Copyright © 2009, Maxim Integrated Products
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
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
:Version: 1.0

:Todo:
 - Implement support of configuration file

:Author: Grégory Romé - gregory.rome@maxim-ic.com
:Organization: Maxim Integrated Products
:Copyright: Copyright © 2009, Maxim Integrated Products
:License: BSD License - http://www.opensource.org/licenses/bsd-license.php
"""

__version__ = "$Revision: 718 $"
# $Source$

#---- IMPORTS

#import pdb

import sys
import os
import re
from optparse import OptionParser
from time import sleep

from BootloaderScp import BootloaderScp, ScpCmd
from scan import scan

#---- CONSTANTS

AUTHOR = u"Grégory Romé"

VERSION = "1.2"

PROG = "serial_sender"

COPYRIGHT = u"Copyright © 2009-2010, Maxim Integrated Products"

LICENSE = u"""Copyright © 2009-2010, Maxim Integrated Products
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

#---- GLOBALS

_verbose = False
_extra_verbose = True

#---- METHODS

def parse_scpcmd_file(cmds_file, bl_scp, bl_emulation):
	""" Parse the commands file

	:Parameters:
		cmds_file
			the commands file
		bl_scp
			BootloaderScp object
		bl_emulation
			Emulation mode (True/False)
	"""

	if bl_emulation:
		pck_filter = 'bl'
		print '[INFO] Emulation mode'
	else:
		pck_filter = 'host'


	#pdb.set_trace();

	print 'Start SCP session (use -v for details)'
	for line in cmds_file:

		# Read command
		file_name = line.strip()

		result = re.search(pck_filter, file_name)
		m = re.search('(\w+[_-]*\w+)\.(\d+)\.(\w+[_-]*\w+)\.((\w+[_-]*)*)\.\w+', file_name)

		if m is not None:
			id = m.group(2)
			cmd = m.group(4)
		else:
			print >> sys.stderr, "error: wrong filename: " + file_name
			raise Exception()
			break

		if result is None:
			#tries=15
			tries=1
			while tries:
				
				if _verbose:
					print id + ' WAIT< ' + cmd
				try:
					scp_cmd = bl_scp.readPacket()
					break
				except:
					print >> sys.stderr, "error: read packet error occur. #" , tries
					tries = tries-1
					if tries == 0:
						raise Exception()
					else:
						# Re-send last packet
						wtries=15
						while wtries:
							packet = open(last_file, 'rb')
							if _verbose:
								print id + ' SEND> ' + cmd
							try:
								bl_scp.write(packet.read())
								bl_scp.flush()
								break
							except:
								wtries=wtries-1
								bl_scp.flushOutput()
							   # bl_scp.close()
								bl_scp = BootloaderScp(port_name, options.timeout)
								if wtries==0:
									raise Exception()

						packet.close()

			if scp_cmd is None:
				print >> sys.stderr, "error: receiving packet failed"
				raise Exception()
				break

			packet = open(file_name, 'rb')
			expected_cmd = ScpCmd(packet)

			if expected_cmd is None:
				print >> sys.stderr, "error: unable to read file packet"
				raise Exception()
				break

			packet.close()
			if expected_cmd != scp_cmd:
				msg = "error: received packet is not the expected one"
				print >> sys.stderr, msg
				if _verbose:
					print "======================================"
					print "Expected Command"
					print expected_cmd
					print "--------------------------------------"
					print "Received Command"
					print scp_cmd
					print "======================================"
				if cmd != 'hello_reply':
					raise Exception()
					break
		else:
			last_file=file_name

			wtries=5
			while wtries:
				packet = open(file_name, 'rb')
				if _verbose:
					print id + ' SEND> ' + cmd
				try:
					bl_scp.write(packet.read())
					bl_scp.flush()
					break
				except:
					wtries=wtries-1
#                    bl_scp.close()
					bl_scp = BootloaderScp(port_name, options.timeout)
					if wtries==0:                   
						raise Exception()
			
			packet.close()


#---- MAIN

if __name__ == "__main__":

	m = re.search('Rev(ision)*\s*:\s*(\d+)', __version__)
	if m is not None:
		revision = m.group(2)

	usage = "usage: " + PROG + " [options] FILENAME"
	version = "%prog " + VERSION + '-rev' + revision + "\n" + COPYRIGHT
	parser = OptionParser(prog=PROG, usage=usage, version=version,
						  epilog=EPILOG)
	parser.add_option("-s", "--serial", dest="serial", type="string",
					  help="define the serial port to use")
	parser.add_option("-v", "--verbose", action="store_true", dest="verbose",
					  default=False, help="enable verbose mode")
	parser.add_option("-l", "--license", action="store_true", dest="license",
					  default=False, help="display license")
	parser.add_option("--list-serial", action="store_true", dest="list_serial",
					  default=False, help="display available serial ports")
	parser.add_option("-b", "--bl-emulation", action="store_true",
					  dest="bl_emulation", default=False,
					  help="emulate the bootloader")
	parser.add_option("-t", "--timeout", dest="timeout", type="int",
						default=35, help="specifies the protocol timeout (s). \
By default the timeout is 35s")

	(options, args) = parser.parse_args()

	if options.license == True:
		print LICENSE
		sys.exit(0)

	if options.list_serial == True:
		print "Available serial ports:"
		for port_name in scan():
			print '  - ' + port_name
		sys.exit(0)

	if args.__len__() != 1:
		parser.error("argument(s) missing")
		sys.exit(1)

	filename = args[0]

	if options.verbose:
		_verbose = True

	if options.serial is None:
		if os.name == 'nt':
			serial = 'COM1'
		else:
			serial = '/dev/ttyS0'
	else:
		serial = options.serial

	serial_exists = False
	for port_name in scan():
		if port_name == serial:
			serial_exists = True
			break;

	if serial_exists == False:
		print >> sys.stderr, "error: invalid serial port"
		sys.exit(5)

	if options.verbose:
		print 'Open file: ' + filename
	cmds_file = open(filename, 'r')

	if options.verbose:
		print 'Open serial port: ' + port_name + ' (timeout: ' + str(options.timeout) + 's)'
	bl_scp = BootloaderScp(port_name, options.timeout)
   # bl_scp.read(100)

	try:
		parse_scpcmd_file(cmds_file, bl_scp, options.bl_emulation)
		print >> sys.stderr, "SCP session OK"
	except:
		print >> sys.stderr, "error: SCP session FAILED"
		sys.exit(1)
	finally:
		cmds_file.close()
		bl_scp.close()

	sys.exit(0)
