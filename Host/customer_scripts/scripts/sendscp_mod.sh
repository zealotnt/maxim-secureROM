#!/bin/sh

# Copyright (C) 2012-2014 Maxim Integrated Products, Inc., All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
# OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# Except as contained in this notice, the name of Maxim Integrated 
# Products, Inc. shall not be used except as stated in the Maxim Integrated 
# Products, Inc. Branding Policy.
#
# The mere transfer of this software does not imply any licenses
# of trade secrets, proprietary technology, copyrights, patents,
# trademarks, maskwork rights, or any other form of intellectual
# property whatsoever. Maxim Integrated Products, Inc. retains all 
# ownership rights.

usage() {
	echo " Syntax: sendscp.sh <serialport> <input_dir>"
	echo "    <serialport> = serial port device (e.g. /dev/ttyS0 (linux) or COM1 (windows))"
	echo "    <input_dir> = directory that contains the SCP packet list"
	echo "Note: "
	echo "See also: build_application.sh to build an SCP script."
}

TOOLDIR=$(readlink -e $(dirname $0))
bFolder=0
bCompress=0

case $# in
2)	readonly serialport=$1
	readonly inputCompressed=$2
	;;
*)	usage  >&2
	exit 2
	;;
esac

sync
echo ""
if [ -d "buildSCP/$inputCompressed" ]; then
	bFolder=1
else
	if [ ! -e "buildSCP/$inputCompressed" ]; then
	echo "Error:: the <input_compressed_file> ($inputCompressed) does not exist."
	exit 1
	fi
	bCompress=1
fi


#Uncompress file
if [ $bCompress == 1 ]; then
	input=${inputCompressed%.tar.gz}
	tar -xvf "$inputCompressed"
	cd -- "buildSCP/$input"  ||  exit
elif [ $bFolder == 1 ]; then
	input=$2
	cd -- "buildSCP/$input"  ||  exit
else
	echo "Input expected !!"
	exit 1
fi

#identifying the OS, as cygwin uses a serial_sender.exe while linux directly uses the python application
system=$(uname -s)
case $system in
*CYGWIN*) readonly serial_sender_bin=serial_sender.exe
	;;
*Linux*)  
	readonly serial_sender_bin=serial_sender.py
	if [ ! -e $serialport ]; then
	  echo "Error:: $serialport: invalid. Make sure the serial port exists."
	  exit 1
	fi
	;;
*)	echo >&2 "Unknown system \`$system'!"
	exit 1
	;;
esac

if [ ! -f packet.list ]; then
echo "Error:: the <input_dir> ($input) does not seem to contain a SCP script."
exit 1
fi

echo "Ready to execute $(readlink -e .)"
read -p "Power cycle the MAX32550 system then press [Enter] IMMEDIATELY!" reply
echo "Please wait..."
#Waiting to avoid the USB SCP time window (4s by default)
sleep 4

$TOOLDIR/../lib/serial_sender/$serial_sender_bin -s$serialport -t 2 -v packet.list

if [ $? -ne 0 ] ; then
echo "ERROR."
echo "Make sure you pressed [Enter] right after powering-up the system."
echo "Make sure you have no terminal opened on $serialport."
exit 1
fi

echo "SUCCESS."