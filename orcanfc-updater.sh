#!/bin.bash


##################################################################################################
# constant definition
##################################################################################################
UPGRADE_TYPE_LIST=("ERASER" "SURIBL" "SURISDK" "ALL")
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cmdname=$(basename $0)
. $DIR/Host/customer_scripts/scripts/colorCode.sh

# Updater/Application to update
PACKAGE_SVC=$DIR/Host/surisdk-fw-upgrade/svc
DEFAULT_SVC=/home/root/ischool/svc

SURISCP_UPDATER=$DIR/Host/customer_scripts/lib/serial_sender/serial_sender.py
SURISDK_UPDATER_PC=$DIR/Host/surisdk-fw-upgrade/orcanfc_updater
SURISDK_UPDATER_BOARD=$DIR/Host/surisdk-fw-upgrade/orcanfc_board_updater
SURISCP_FIRST_TRY=100

# Detect environment, and use the updater accordingly
IsBoard=`cat /proc/cpuinfo | grep "model name" | grep "ARM"`
SURISDK_UPDATER=""
SURISDK_BOARD_TOGGLE_RST=""
MAXIM_RST_PIN="81"

# Places to find firmware
SURI_ERASER_DIR=$DIR/Host/customer_scripts/scripts/buildSCP/eraser
SURIBL_FW_DIR=$DIR/Host/customer_scripts/scripts/buildSCP/suribl
SURI_OTP_DIR=$DIR/Host/customer_scripts/scripts/buildSCP/OTP_UART_250ms
SURI_KEY_DIR=$DIR/Host/customer_scripts/scripts/buildSCP/prod_p3_write_crk

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
	echo "  -p PORT | --port=PORT       Serial device to interface with reader"
	echo "                              ex, -p /dev/ttyUSB0"
	echo "  -t | --type=UPGRADE_TYPE    Upgrade firmware type"
	echo "                              support types: '${UPGRADE_TYPE_LIST[@]}', if no type specify, 'ALL' type will be default"
	echo "  -f | --file=SURISDK         Location of surisdk firmware, should be binary/json file"
	echo "  -h | --help                 Show this message"
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

	echoerr "Not regconize upgrade type '$1'"
	exit 2
}

UpdateFirmwareSCP()
{
	retVal=0
	curDir=$(pwd)
	cd $1

	if [ ! -f packet.list ]; then
		echoerr "Error: the <input_suribl_dir> does not seem to contain a SCP script."
		retVal=1
	fi

	if [[ "$IsBoard" != "" ]]; then
		echo "Killall pending svc to upgrade Maxim through SCP"
		killall svc
	fi

	python $SURISCP_UPDATER -s $SERIAL_PORT -t 2 -v -f $SURISCP_FIRST_TRY -w packet.list $SURISDK_BOARD_TOGGLE_RST
	scpRet=$?
	case $scpRet in
	0)
		;;
	1)	echoerr "Restrict data, Key/OTP settings already loaded"
		retVal=1
		;;
	*)	echoerr "Suribl flash fail"
		retVal=2
		;;
	esac

	cd $curDir
	return $retVal
}

EraseMaximFlash()
{
	echonoti "**************************************************"
	echonoti "Erase flash of Maxim"
	UpdateFirmwareSCP $SURI_ERASER_DIR
	retVal=$?
	echonoti "**************************************************"
	return $retVal
}

LoadMaximKey()
{
	echonoti "**************************************************"
	echonoti "Load Key of Maxim"
	UpdateFirmwareSCP $SURI_KEY_DIR
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
	UpdateFirmwareSCP $SURI_OTP_DIR
	retVal=$?
	# load otp could fail, if it already loaded otp, so continue if load otp fail
	if [[ $retVal == 1 ]]; then
		retVal=0
	fi
	echonoti "**************************************************"
	return $retVal
}

UpgradeSuribl()
{
	echonoti "**************************************************"
	echonoti "Update Maxim 2nd bootloader"
	UpdateFirmwareSCP $SURIBL_FW_DIR
	retVal=$?
	echonoti "**************************************************"
	return $retVal
}

UpgradeSurisdk()
{
	echonoti "**************************************************"
	echonoti "Update Maxim surisdk firmware, using $UPGRADE_FILE"

	if [[ "$IsBoard" != "" ]]; then
		killall svc
		echoinfo "Kill others svc and start another svc session"
		echoinfo "Set STYL_SVC_RF_CMD ENV_VAR to $SERIAL_PORT"
		export STYL_SVC_RF_CMD=$SERIAL_PORT
		if [[ -f "$PACKAGE_SVC" ]]; then
			echoinfo "Run default svc at $PACKAGE_SVC"
			$PACKAGE_SVC &
		else
			echoinfo "Run platform svc at $DEFAULT_SVC"
			$DEFAULT_SVC &
		fi
	fi

	$SURISDK_UPDATER $SERIAL_PORT $UPGRADE_FILE
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

if [[ "$UPGRADE_TYPE" == "" ]]; then
	echoinfo "No upgrade firmware method specified, use upgrade 'ALL' as default"
	UPGRADE_TYPE="ALL"
fi

if [[ "$UPGRADE_TYPE" == "ALL" || "$UPGRADE_TYPE" == "SURISDK" ]]; then
	if [[ ! -f "$UPGRADE_FILE" ]]; then
		echoerr "surisdk firmware file '$UPGRADE_FILE' not found"
		usage
		exit 1
	fi
fi

CheckValidFileType $UPGRADE_TYPE "${UPGRADE_TYPE_LIST[@]}"

if [[ "$IsBoard" == "" ]];then
	echoinfo "Environment PC detected"
	SURISDK_UPDATER=$SURISDK_UPDATER_PC
else
	echoinfo "Environment Board detected"
	SURISDK_UPDATER=$SURISDK_UPDATER_BOARD
	SURISDK_BOARD_TOGGLE_RST=" --resetMaxim "
	echoinfo "Export Maxim RESET Pin"
	echoinfo $MAXIM_RST_PIN > /sys/class/gpio/export
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

if [[ "$UPGRADE_TYPE" == "ERASER" || "$UPGRADE_TYPE" == "ALL" ]]; then
	EraseMaximFlash
	if [[ $? != 0 ]]; then
		exit 1
	fi
fi

if [[ "$UPGRADE_TYPE" == "SURIBL" || "$UPGRADE_TYPE" == "ALL" ]]; then
	UpgradeSuribl
	if [[ $? != 0 ]]; then
		exit 1
	fi
fi

if [[ "$UPGRADE_TYPE" == "SURISDK" || "$UPGRADE_TYPE" == "ALL" ]]; then
	UpgradeSurisdk
	if [[ $? != 0 ]]; then
		exit 1
	fi
fi
