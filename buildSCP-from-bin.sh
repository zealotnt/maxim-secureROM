#!/bin/bash


##################################################################################################
# constant definition
##################################################################################################
UPDATER_VERSION="0.0.5 NETS KEY"
UPGRADE_TYPE_LIST=("ORCANFC" "ALL")
DEFAULT_UPG_TYPE="ORCANFC"

CURDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cmdname=$(basename $0)
. $CURDIR/Host/customer_scripts/scripts/colorCode.sh

ORCANFC_SCP_UPDATER=$CURDIR/Host/customer_scripts/lib/serial_sender/serial_sender.py
ORCANFC_SCP_FIRST_TRY=100

# Detect environment, and use the updater accordingly
IsBoard=`cat /proc/cpuinfo | grep "model name" | grep "ARM"`
ORCANFC_BOARD_TOGGLE_RST=""
MAXIM_RST_PIN="81"

# Places to find firmware
ORCANFC_TEMP_EXTRACT_FOLDER=/tmp/orcanfc_firmware
ORCANFC_FW_DIR=""
ORCANFC_OTP_DIR=$CURDIR/Host/customer_scripts/scripts/buildSCP/OTP_UART_250ms
ORCANFC_KEY_DIR=$CURDIR/Host/customer_scripts/scripts/buildSCP/prod_p3_write_crk

SCRIPT_NAME=`basename "$0"`
SCRIPT_HDR="[$SCRIPT_NAME]"


##################################################################################################
# function definition
##################################################################################################
echoinfo() { if [[ $QUIET -ne 1 ]]; then echo -e "$SCRIPT_HDR $@${KRESET}${ENDL}" 1>&2; fi }
echoerr() { if [[ $QUIET -ne 1 ]]; then echo -e "${KBOLD}${KLRED}$SCRIPT_HDR $@${KRESET}${ENDL}" 1>&2; fi }
echonoti() { if [[ $QUIET -ne 1 ]]; then echo -e "${KBOLD}${KLGRN}$SCRIPT_HDR $@${KRESET}" 1>&2; fi }

usage() {
	echo "Usage:"
	echo "  bash $cmdname [-f BINARY_FILE] [-o OUTPUT_FOLDER]"
	echo "  -f BINARY_FILE                 Binary .bin file to be signed"
	echo "  -o OUTPUT-FOLDER               SCP output folder"
	echo "  -h | --help                    Show this message"
	exit 1
}


main() {
	##################################################################################################
	# main program
	##################################################################################################
	# process arguments
	while [[ $# -gt 0 ]]
	do
		echonoti "1:'$1'"
		echonoti "1:'$2'"
		case "$1" in
			-f)
			BINARY_FILE="$2"
			if [[ $BINARY_FILE == "" ]]; then echoerr "-f argument required"; usage; fi
			shift 2
			;;
			-o)
			OUTPUT_FOLDER="$2"
			if [[ $OUTPUT_FOLDER == "" ]]; then echoerr "-o argument required"; usage; fi
			shift 2
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

	if [ ! -d $OUTPUT_FOLDER ]; then
		mkdir -p $OUTPUT_FOLDER
	fi

	set -x
	OUTPUT_FOLDER_ABS=$(realpath $OUTPUT_FOLDER)
	BINARY_FILE_NAME=$(basename $BINARY_FILE)
	BINARY_FILE_NAME=${BINARY_FILE_NAME%.bin}

	cp $BINARY_FILE $CURDIR/Host/customer_scripts/CaSign/
	cd $CURDIR/Host/customer_scripts/CaSign/

	./ca_sign_build algo=ecdsa ca="$BINARY_FILE_NAME.bin" sca="$BINARY_FILE_NAME.signed.bin" \
	load_address=10000000 jump_address=10000020 arguments= version=01000003 application_version=01010000 verbose=no
	cp "$BINARY_FILE_NAME.signed.bin" ../scripts
	cd ../scripts
	bash buildSCP_mod.sh "$BINARY_FILE_NAME.signed.bin" "$BINARY_FILE_NAME.signed.scp" $OUTPUT_FOLDER_ABS
}

main $@


