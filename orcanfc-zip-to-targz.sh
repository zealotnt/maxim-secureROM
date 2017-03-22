#!/bin/bash

##################################################################################################
# constant definition
##################################################################################################
ORCANFC_CONVERT_EXTRACT_FOLDER=/tmp/temp_convert_compress
FOLDER_NAME_CONVERT_TO=tempFw

cmdname=$(basename $0)
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. $DIR/Host/customer_scripts/scripts/colorCode.sh

##################################################################################################
# function definition
##################################################################################################
echoinfo() { if [[ $QUIET -ne 1 ]]; then echo -e "$SCRIPT_HDR $@${KRESET}${ENDL}" 1>&2; fi }
echoerr() { if [[ $QUIET -ne 1 ]]; then echo -e "${KBOLD}${KLRED}$SCRIPT_HDR $@${KRESET}${ENDL}" 1>&2; fi }
echonoti() { if [[ $QUIET -ne 1 ]]; then echo -e "${KBOLD}${KLGRN}$SCRIPT_HDR $@${KRESET}" 1>&2; fi }

usage()
{
	echo "Usage:"
	echo "  bash $cmdname <zip-file-to-convert>"
	exit 1
}

if [[ $# != 1 ]]; then
	usage
	exit 1
fi

CheckPackageInstall()
{
	# [Ref](http://stackoverflow.com/questions/1298066/check-if-a-package-is-installed-and-then-install-it-if-its-not)
	package=$1
	status=$(dpkg-query -W -f='${Status}' $package 2>/dev/null | grep -c "ok installed")
	return $status
}

CheckAndInstallUnzip()
{
	CheckPackageInstall unzip
	retVal=$?
	if [[ $retVal == 0 ]]; then
		echonoti "Unzip not installed, install it"
		sudo apt-get install unzip
	fi
}
##################################################################################################
# main program
##################################################################################################
UPGRADE_FILE=$1
UPGRADE_FILE_NAME=$(basename $UPGRADE_FILE)
# Remove .zip if existed
UPGRADE_FILE_NAME=${UPGRADE_FILE_NAME%.zip}
echo $UPGRADE_FILE_NAME
if [[ -f "$UPGRADE_FILE" ]]; then
	rm -rf $ORCANFC_CONVERT_EXTRACT_FOLDER
	mkdir -p $ORCANFC_CONVERT_EXTRACT_FOLDER

	# Check if the file is "zip" or "tar.gz"
	if [[ ${UPGRADE_FILE: -4} == ".zip" ]]; then
		CheckAndInstallUnzip
		unzip $UPGRADE_FILE -d $ORCANFC_CONVERT_EXTRACT_FOLDER
	else
		echoerr "Error: $UPGRADE_FILE: file type not regconize"
		exit 1
	fi

	# Detect the extracted folder
	outputFolder=`ls $ORCANFC_CONVERT_EXTRACT_FOLDER`
	ORCANFC_FW_DIR=$ORCANFC_CONVERT_EXTRACT_FOLDER/$outputFolder

	# Convert the extract folder to $FOLDER_NAME_CONVERT_TO folder name
	mv $ORCANFC_FW_DIR $ORCANFC_CONVERT_EXTRACT_FOLDER/$FOLDER_NAME_CONVERT_TO

	# cd to that folder to avoid multilevel folder structure in a compress file
	cd $ORCANFC_CONVERT_EXTRACT_FOLDER

	# Compress it to tar.gz file format
	TAR_GZ_OUTPUT=$UPGRADE_FILE_NAME.tar.gz
	tar -zcf $DIR/$TAR_GZ_OUTPUT $FOLDER_NAME_CONVERT_TO
	echonoti "Successfully convert $UPGRADE_FILE to $TAR_GZ_OUTPUT"
else
	echoerr "File '$UPGRADE_FILE' not found"
	usage
	exit 1
fi
