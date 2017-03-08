#!/bin.bash


##################################################################################################
# constant definition
##################################################################################################
UPGRADE_TYPE_LIST=("ORCANFC" "ALL")
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cmdname=$(basename $0)
. $DIR/Host/customer_scripts/scripts/colorCode.sh

ORCANFC_SCP_UPDATER=$DIR/Host/customer_scripts/lib/serial_sender/serial_sender.py
ORCANFC_SCP_FIRST_TRY=100

# Detect environment, and use the updater accordingly
IsBoard=`cat /proc/cpuinfo | grep "model name" | grep "ARM"`
ORCANFC_BOARD_TOGGLE_RST=""
MAXIM_RST_PIN="81"

# Places to find firmware
ORCANFC_TEMP_EXTRACT_FOLDER=/tmp/orcanfc_firmware
ORCANFC_FW_DIR=""
ORCANFC_OTP_DIR=$DIR/Host/customer_scripts/scripts/buildSCP/OTP_UART_250ms
ORCANFC_KEY_DIR=$DIR/Host/customer_scripts/scripts/buildSCP/prod_p3_write_crk

SCRIPT_NAME=`basename "$0"`
SCRIPT_HDR="[$SCRIPT_NAME]"

##################################################################################################
# function definition
##################################################################################################
echoinfo() { if [[ $QUIET -ne 1 ]]; then echo -e "$SCRIPT_HDR $@${KRESET}${ENDL}" 1>&2; fi }
echoerr() { if [[ $QUIET -ne 1 ]]; then echo -e "${KBOLD}${KLRED}$SCRIPT_HDR $@${KRESET}${ENDL}" 1>&2; fi }
echonoti() { if [[ $QUIET -ne 1 ]]; then echo -e "${KBOLD}${KLGRN}$SCRIPT_HDR $@${KRESET}" 1>&2; fi }

usage()
{
	echo "Usage:"
	echo "  bash $cmdname [-p COMPORT] [-t upgrade-type]"
	echo "  -p PORT | --port=PORT          Serial device to interface with reader"
	echo "                                 ex, -p /dev/ttyUSB0"
	echo "  -t | --type=UPGRADE_TYPE       Upgrade firmware type"
	echo "                                 support types: '${UPGRADE_TYPE_LIST[@]}', if no type specify, 'ALL' type will be default"
	echo "  -f | --file=ORCANFC_FIRMWARE   Location of orcanfc firmware, should be tar/zip file, or folder contains SCP package"
	echo "  -h | --help                    Show this message"
	exit 1
}

CheckValidFileType()
{
	#function usage:
	arguments=("$@")
	arguments_len=${#arguments[@]}
	array=("${arguments[@]:1:$arguments_len}")
	count=${#array[@]}
	for ((j=0; j < $count; j++)); do
		if [[ "$1" == ${array[j]} ]];then
			return
		fi
	done

	usage
	echoerr "Not regconize upgrade type '$1'"
	exit 2
}

UpdateFirmwareSCP()
{
	retVal=0
	curDir=$(pwd)

	# Cd to the SCP firmware folder
	cd $1

	# Check if we need to lengthen the first try timing
	firstTry=$2
	if [[ $firstTry == "yes" ]]; then
		firstTryParam="-f $ORCANFC_SCP_FIRST_TRY"
	else
		firstTryParam=""
	fi

	ls -1 *.packet >packet.list
	echo "Update SCP with package $1"

	if [ ! -f packet.list ]; then
		echoerr "Error: the <input_scp_dir> does not seem to contain a SCP script."
		retVal=1
	fi

	python $ORCANFC_SCP_UPDATER -s $SERIAL_PORT -t 2 -v $firstTryParam -w packet.list $ORCANFC_BOARD_TOGGLE_RST
	scpRet=$?
	case $scpRet in
	0)
		;;
	1)	echoerr "Restrict data, Key/OTP settings already loaded"
		retVal=1
		;;
	*)	echoerr "Update using SCP fail"
		retVal=2
		;;
	esac

	cd $curDir
	return $retVal
}

LoadMaximKey()
{
	echonoti "**************************************************"
	echonoti "Load Key of Maxim"
	UpdateFirmwareSCP $ORCANFC_KEY_DIR "yes"
	retVal=$?
	# load key could fail, if it already loaded key, so continue if load key fail
	if [[ $retVal == 1 ]]; then
		retVal=0
	fi
	echonoti "**************************************************"
	return $retVal
}

LoadMaximOTP()
{
	echonoti "**************************************************"
	echonoti "Load OTP of Maxim"
	UpdateFirmwareSCP $ORCANFC_OTP_DIR "yes"
	retVal=$?
	# load otp could fail, if it already loaded otp, so continue if load otp fail
	if [[ $retVal == 1 ]]; then
		retVal=0
	fi
	echonoti "**************************************************"
	return $retVal
}

UpgradeOrcaNfc()
{
	echonoti "**************************************************"
	echonoti "Update OrcaNFC Firmware"
	UpdateFirmwareSCP $ORCANFC_FW_DIR "no"
	retVal=$?
	echonoti "**************************************************"
	return $retVal
}

##################################################################################################
# main program
##################################################################################################
# process arguments
while [[ $# -gt 0 ]]
do
	case "$1" in
		-p)
		SERIAL_PORT="$2"
		if [[ $SERIAL_PORT == "" ]]; then echoerr "-p argument required"; usage; fi
		shift 2
		;;
		--port=*)
		SERIAL_PORT="${1#*=}"
		shift 1
		;;
		-t)
		UPGRADE_TYPE="$2"
		if [[ $UPGRADE_TYPE == "" ]]; then echoerr "-t argument required"; usage; fi
		shift 2
		;;
		--type=*)
		UPGRADE_TYPE="${1#*=}"
		shift 1
		;;
		-f)
		UPGRADE_FILE="$2"
		if [[ $UPGRADE_FILE == "" ]]; then echoerr "-f argument required"; usage; fi
		shift 2
		;;
		--file=*)
		UPGRADE_FILE="${1#*=}"
		shift 1
		;;
		-h | --help)
		usage
		;;
		*)
		echoerr "Unknown argument: '$1'"
		usage
		;;
	esac
done

if [[ "$SERIAL_PORT" == "" ]]; then
	echoerr "Error: you need to provide an available serial port to continue."
	usage
fi

if [[ "$UPGRADE_FILE" == "" ]]; then
	echoerr "Error: you need to provide a valid orcanfc firmware to continue."
	usage
fi

if [[ "$UPGRADE_TYPE" == "" ]]; then
	echoinfo "No upgrade firmware method specified, use upgrade 'ORCANFC' as default"
	UPGRADE_TYPE="ORCANFC"
fi

if [[ "$UPGRADE_TYPE" == "ALL" || "$UPGRADE_TYPE" == "ORCANFC" ]]; then
	# Check if input firmware is a file
	if [[ -f "$UPGRADE_FILE" ]]; then
		rm -rf $ORCANFC_TEMP_EXTRACT_FOLDER
		mkdir -p $ORCANFC_TEMP_EXTRACT_FOLDER
		tar -xf $UPGRADE_FILE -C $ORCANFC_TEMP_EXTRACT_FOLDER
		ORCANFC_FW_DIR=$ORCANFC_TEMP_EXTRACT_FOLDER/tempFw
	# Check if input param is a folder
	elif [[ -d "$UPGRADE_FILE" ]]; then
		ORCANFC_FW_DIR=$UPGRADE_FILE
	else
		echoerr "OrcaNfc firmware file '$UPGRADE_FILE' not found"
		usage
		exit 1
	fi
fi

CheckValidFileType $UPGRADE_TYPE "${UPGRADE_TYPE_LIST[@]}"

if [[ "$IsBoard" == "" ]];then
	echoinfo "Environment PC detected"
else
	echoinfo "Environment Board detected"
	ORCANFC_BOARD_TOGGLE_RST=" --resetMaxim "
	echoinfo "Export Maxim RESET Pin"
	echo $MAXIM_RST_PIN > /sys/class/gpio/export
fi

if [[ "$UPGRADE_TYPE" == "ALL" ]]; then
	LoadMaximKey
	if [[ $? != 0 ]]; then
		exit 1
	fi
	LoadMaximOTP
	if [[ $? != 0 ]]; then
		exit 1
	fi
fi

if [[ "$UPGRADE_TYPE" == "ORCANFC" || "$UPGRADE_TYPE" == "ALL" ]]; then
	UpgradeOrcaNfc
	if [[ $? != 0 ]]; then
		exit 1
	fi
fi
