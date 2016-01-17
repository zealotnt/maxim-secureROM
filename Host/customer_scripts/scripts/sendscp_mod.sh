#!/bin/bash
# Import color code variables
. ./colorCode.sh

usage() 
{
	echo " Syntax: sendscp.sh <serialport> <input_dir> <Toogle>"
	echo "    <serialport> = serial port device (e.g. //dev//ttyS0 (linux) or COM1 (windows))"
	echo "    <input_dir> = directory that contains the SCP packet list"
	echo "    <Toogle> = accept 'y//n', y for toggle reset GPIO, n for manually reset it"
	echo "Note: "
	echo "See also: build_application_mod.sh to build an SCP script."
}

TOOLDIR=$(readlink -e $(dirname $0))
bFolder=0
bCompress=0

case $# in
2)	readonly serialport=$1
	readonly inputCompressed=$2
	readonly bToogleGPIO='n'
	;;
3)	readonly serialport=$1
	readonly inputCompressed=$2
	readonly bToogleGPIO=$3
	;;
*)	usage  >&2
	exit 2
	;;
esac

if [[ ($bToogleGPIO != 'y') || ($bToogleGPIO != 'n') ]]; then
	echo "param <Toogle> not invalid, only 'y/n' expected"
	exit 2
fi

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


if [ $bToogleGPIO == 'y' ]; then
	echo "{KRED}Going to enable GPIO81 of iMX6{KRESET}"
	echo 81 > /sys/class/gpio/export
	echo "{KRED}Pull the GPIO_Reset pin low{KRESET}"
	echo low > /sys/class/gpio/gpio81/direction
fi

echo "Ready to execute $(readlink -e .)"
read -p "{KLRED}{KBOLD}Power cycle the MAX32550 system then press [Enter] IMMEDIATELY!{KRESET}" reply
echo "Please wait..."
#Waiting to avoid the USB SCP time window (4s by default)
timeSleep=3.5

if [ $bToogleGPIO == 'y' ]; then
	echo "{KRED}Pull the GPIO_Reset pin high again, wait $timeSleep second{KRESET}"
	echo high > /sys/class/gpio/gpio81/direction
fi

sleep $timeSleep


$TOOLDIR/../lib/serial_sender/$serial_sender_bin -s$serialport -t 2 -v packet.list

if [ $? -ne 0 ] ; then
echo "ERROR."
echo "Make sure you pressed [Enter] right after powering-up the system."
echo "Make sure you have no terminal opened on $serialport."
exit 1
fi

echo "SUCCESS."
