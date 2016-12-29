#!/bin.bash


##################################################################################################
# constant definition
##################################################################################################
UPGRADE_TYPE_LIST=("ERASER" "SURIBL" "SURISDK" "ALL")
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cmdname=$(basename $0)
. $DIR/Host/customer_scripts/scripts/colorCode.sh

SURISCP_UPDATER=$DIR/Host/customer_scripts/lib/serial_sender/serial_sender.py
SURISDK_UPDATER=$DIR/Host/surisdk-fw-upgrade/surisdk_lite_updater
SURISCP_FIRST_TRY=100

SURI_ERASER_DIR=$DIR/Host/customer_scripts/scripts/buildSCP/eraser
SURIBL_FW_DIR=$DIR/Host/customer_scripts/scripts/buildSCP/suribl-lite

##################################################################################################
# function definition
##################################################################################################
echoerr() { if [[ $QUIET -ne 1 ]]; then echo -e "${KBOLD}${KLRED}$@${KRESET}${ENDL}" 1>&2; fi }
echonoti() { if [[ $QUIET -ne 1 ]]; then echo -e "${KBOLD}${KLGRN}$@${KRESET}" 1>&2; fi }

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

	python $SURISCP_UPDATER -s $SERIAL_PORT -t 2 -v -f $SURISCP_FIRST_TRY -w packet.list
	scpRet=$?
	case $scpRet in
	0)
		;;
	1)	echoerr "Restrict data, Key/OTP settings already loaded"
		retVal=2
		;;
	*)	echoerr "Suribl flash fail"
		retVal=3
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
	echo "No upgrade firmware method specified, use upgrade 'ALL' as default"
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
