#!/bin/bash
# Import color code variables

# Error code returned:
# 2: "unexpected parameter to scripts"
# 3: "param <Toogle> not invalid, only 'y/n' expected"
# 4: "Error:: the <input_compressed_file> ($inputCompressed) does not exist."
# 5: "Input param expected !!"
# 6: "Error:: $serialport: invalid. Make sure the serial port exists."
# 7: "Unknown system \`$system'!"
# 8: "Error:: the <input_dir> ($input) does not seem to contain a SCP script."
# 9: "SCP errorred, out of retries"
# 10: same as 9

currentDir=$(pwd)
testCurrentDir=$(basename $currentDir)
echo "$testCurrentDir"

if [[ ( $testCurrentDir != "scripts" ) && ( $# = 3 ) ]]; then
	echo "******* not standing in .../scripts folder, now use absolute path *******"
	currentDir=/home/root/secureROM-Sirius/Host/customer_scripts/scripts
fi

if [ -f colorCode.sh ]; then
	. ./colorCode.sh
else 
	. $currentDir/colorCode.sh
fi


resetMaxim()
{
	echo -e "${KRED}Going to enable GPIO81 of iMX6${KRESET}"
	if [ ! -d /sys/class/gpio/gpio81 ]; then
		echo "Export gpio81 for manually usage"
		echo 81 > /sys/class/gpio/export
	else
		echo "Already exported"
	fi
	echo -e "${KRED}Pull the GPIO_Reset pin high${KRESET}"
	echo high > /sys/class/gpio/gpio81/direction
	sleep 1
	echo -e "${KRED}Pull the GPIO_Reset pin low again ${KRESET}"
	echo low > /sys/class/gpio/gpio81/direction
}

usage()
{
	echo " Syntax: sendscp.sh <serialport> <input_dir> <Toogle>"
	echo "    <serialport> = serial port device (e.g. //dev//ttyS0 (linux) or COM1 (windows))"
	echo "    <input_dir> = directory that contains the SCP packet list"
	echo "    <Toogle> = accept 'y//n', y for toggle reset GPIO, n for manually reset it"
	echo "Note: "
	echo "See also: build_application_mod.sh to build an SCP script."
}

if [ $# != 3 ]; then
	TOOLDIR=$(readlink -e $(dirname $0))
else
	TOOLDIR=$currentDir
	echo -e "${KRED}Run on Sirius platform, set up TOOLDIR as current directory${KRESET}"
fi

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

if [[ ($bToogleGPIO != 'y') && ($bToogleGPIO != 'n') ]]; then
	echo "param <Toogle> not invalid, only 'y/n' expected"
	exit 3
fi

sync
echo ""
if [ -d "$inputCompressed" ]; then
	bFolder=1
else
	if [ ! -e "$inputCompressed" ]; then
	echo "Error:: the <input_compressed_file> ($inputCompressed) does not exist."
	exit 4
	fi
	bCompress=1
fi


#Uncompress file
if [ $bCompress == 1 ]; then
	input=${inputCompressed%.tar.gz}
	tar -xvf "$inputCompressed"
	cd -- "$input"  ||  exit
elif [ $bFolder == 1 ]; then
	input=$2
	cd -- "$input"  ||  exit
else
	echo "Input param expected !!"
	exit 5
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
	  exit 6
	fi
	;;
*)	echo >&2 "Unknown system \`$system'!"
	exit 7
	;;
esac

if [ ! -f packet.list ]; then
echo "Error:: the <input_dir> ($input) does not seem to contain a SCP script."
exit 8
fi


if [ $bToogleGPIO == 'y' ]; then
	resetMaxim
else
	echo "Ready to execute $(readlink -e .)"
	echo -e "${KLRED}${KBOLD}Power cycle the MAX32550 system now !${KRESET}"
fi

echo "Please wait..."

if [ $bToogleGPIO == 'y' ]; then
	# Add retry mechanism to shell script
	retries=3
	while [ $retries -ne 0 ]; do
		$TOOLDIR/../lib/serial_sender/$serial_sender_bin -s$serialport -t 2 -v packet.list
		case $? in
		0) 	break
			;;
		*)	retries=$((retries-1))
			echo -e "${KRED}Flash fail, try again, $retries times left ${KRESET}"
			resetMaxim
			;;
		esac
	done
else
	# Normally, we don't need to retry so many times when not run with Sirius
	$TOOLDIR/../lib/serial_sender/$serial_sender_bin -s$serialport -t 2 -v packet.list
fi

if [ $? -ne 0 ] ; then
	echo "ERROR."
	echo "Make sure you pressed [Enter] right after powering-up the system."
	echo "Make sure you have no terminal opened on $serialport."
	exit 9
fi

if [ $bToogleGPIO == 'y' ]; then
	if [ $retries -eq 0 ];then
		echo -e "${KRED}${KBOLD}FLASHING FAIL.${KRESET}"
		exit 10
	fi
	
	echo -e "${KRED}${KBOLD}FLASHING SUCCESS.${KRESET}"
	echo "Reseting Maxim"
	resetMaxim
fi

exit 0