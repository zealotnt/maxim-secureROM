#!/bin/bash

##################################################################################################
# constant definition
##################################################################################################
SELF_VERSION="0.0.1"

ENDL="\r\n"
KNRM="\033[0m"
KRED="\033[31m"
KGRN="\033[32m"
KYEL="\033[33m"
KBLU="\033[34m"
KMAG="\033[35m"
KCYN="\033[36m"
KWHT="\033[37m"
KLGRN="\033[92m"
KLRED="\033[91m"
KLYEL="\033[93m"
KLBLU="\033[94m"
KLMAG="\033[95m"
KLCYN="\033[96m"
KBOLD="\033[1m"
KUDLN="\033[4m"
KRESET="\033[0m"
KRBOLD="\033[21m"
SCRIPT_NAME=`basename "$0"`
SCRIPT_HDR="$SCRIPT_NAME"
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

##################################################################################################
# function definition
##################################################################################################
echoinfo() { if [[ $QUIET -ne 1 ]]; then echo -e "[$SCRIPT_HDR-INFO] $@${KRESET}" 1>&2; fi }
echoerr() { if [[ $QUIET -ne 1 ]]; then echo -e "[${KBOLD}${KLRED}$SCRIPT_HDR-ERR${KRESET}] $@${ENDL}" 1>&2; fi }
echonoti() { if [[ $QUIET -ne 1 ]]; then echo -e "[${KBOLD}${KLGRN}$SCRIPT_HDR-NOTI${KRESET}] $@" 1>&2; fi }

cleanProject() {
	echoinfo "Cleaning Project         : \"$2\" Config: \"$3\""
	workspacePath=$1
	projectName=$2
	buildConfigName=$3

	rm -rf "$workspacePath/$projectName/$buildConfigName"

	echonoti "Done cleaning Project    : \"$2\" Config: \"$3\""
}

buildProject() {
	workspacePath=$1
	projectName=$2
	buildConfigName=$3

	cleanProject $1 $2 $3

	echoinfo "Building Project         : \"$2\" Config: \"$3\""

	$ECLIPSE_DIR_PATH/eclipse \
		-nosplash \
		-application org.eclipse.cdt.managedbuilder.core.headlessbuild \
		-data "$workspacePath" \
		-import "$workspacePath/$projectName" \
		-cleanBuild "$projectName/$buildConfigName" \
		-build "$projectName/$buildConfigName" \
		> /dev/null 2>&1

	retVal=$?
	echonoti "Done building Project    : \"$2\" Config: \"$3\" RetVal=$retVal"
	return "$retVal"
}

getOutputFirmware() {
	workspacePath=$1
	projectName=$2
	buildConfigName=$3
	firmwareName=$4
	destination=$5

	echoinfo "Copy: from \"$workspacePath/$projectName/$buildConfigName/$firmwareName\" to $destination"
	cp $workspacePath/$projectName/$buildConfigName/$firmwareName $destination
}

# the function will return these variables
verMajor=""
verMinor=""
verRev=""
getFirmwareVersion() {
	firmwareStr=$1
	fileRead=$2

	if [[ -z "${firmwareStr// }" ]]; then
		item="_"
	else
		item="_${firmwareStr}_"
	fi

	# https://unix.stackexchange.com/questions/84922/extract-a-part-of-one-line-from-a-file-with-sed
	verMajor=$(sed -r -n -e '/^.+FIRMWARE'"$item"'VERSION_MAJOR/ s/^.+FIRMWARE'"$item"'VERSION_MAJOR\s+([0-9]+)/\1/p' ${fileRead})
	verMinor=$(sed -r -n -e '/^.+FIRMWARE'"$item"'VERSION_MINOR/ s/^.+FIRMWARE'"$item"'VERSION_MINOR\s+([0-9]+)/\1/p' ${fileRead})
	verRev=$(sed -r -n -e '/^.+FIRMWARE'"$item"'REVISION/ s/^.+FIRMWARE'"$item"'REVISION\s+([0-9]+)/\1/p' ${fileRead})

	# https://stackoverflow.com/questions/13335516/how-to-determine-whether-a-string-contains-newlines-by-using-the-grep-command
	if [ $(echo "$verMajor" | wc -l) -gt 1 ]; then
		echo "Detect multiple MAJOR_VERSION value line"
		exit -1
	fi

	if [ $(echo "$verMinor" | wc -l) -gt 1 ]; then
		echo "Detect multiple MINOR_VERSION value line"
		exit -1
	fi

	if [ $(echo "$verRev" | wc -l) -gt 1 ]; then
		echo "Detect multiple REVISION_VERSION value line"
		exit -1
	fi
}

setFirmwareVersion() {
	firmwareStr=$1
	fileChange=$2
	newVerMajor=$3
	newVerMinor=$4
	newVerRev=$5

	if [[ -z "${firmwareStr// }" ]]; then
		item="_"
	else
		item="_${firmwareStr}_"
	fi

	# Now we modify the content of file
	sed -r -i 's/(^.+FIRMWARE'"$item"'VERSION_MAJOR\s+)([0-9]+)/\1'"$newVerMajor"'/' ${fileChange}
	sed -r -i 's/(^.+FIRMWARE'"$item"'VERSION_MINOR\s+)([0-9]+)/\1'"$newVerMinor"'/' ${fileChange}
	sed -r -i 's/(^.+FIRMWARE'"$item"'REVISION\s+)([0-9]+)/\1'"$newVerRev"'/' ${fileChange}
}

bResult="n"
check_yn_question() {
	text=$1
	while [ 1 ]; do
		read -p "$text" bResult
		if [ "$bResult" == "y" ]; then
			break
		elif [ "$bResult" == "n" ]; then
			break
		else
			echo -e "Wrong input !!"
		fi
	done
}

usage() {
	echo "Usage:"
	echo "  bash $(basename $0) [-o OUTPUT_FOLDER] [-a VERSION_OFFSET_NUMBER]"
	echo "  -o FOLDER | --out=FOLDER          Folder to contains all of the output firmware"
	echo "  -a NUMBER | --add=NUMBER          Number to offset from the based version"
	echo "  --from-source                     Will build the firmware from source, master branch"
	echo "  -h | --help                       Show this message"
	echo "Script Version: $SELF_VERSION"
	exit 1
}

setConstant() {
	#
	# Script's constant value, not recommend change here
	# if need to overide, set in `setUserConfig` function
	#
	PROJECTS=(
		"xmsdk"
		"xmsdk"
		"surisdk"
		"suribootloader"
	)
	BUILD_CONFIGS=(
		"Release-Board-Slave"
		"Release-Board-Service"
		"Release"
		"Release"
	)
	FIRMWARE_NAME_OUT=(
		"xmsdk"
		"svc"
		"surisdk"
		"suribootloader"
	)
	FIRMWARE_BINARY_EXTENSION_OUT=(
		""
		""
		".bin"
		".bin"
	)
	FIRMWARE_STR=(
		"XMSDK"
		"SVC"
		""
		""
	)
	FIRMWARE_VER_FILE=(
		"src/include/mlsCompileSwitches.h"
		"src/include/mlsCompileSwitches.h"
		"source/styl/include/mlsCompileSwitches.h"
		"source/styl/include/mlsCompileSwitches.h"
	)
	DEST_FOLDER="$DIR/buildOut"
	VER_OFFSET="0"
	FROM_SOURCE="no"
}

setUserConfig() {
	#
	# Each user's specific value, change your values accordingly
	#
	ECLIPSE_DIR_PATH=/home/zealot/bin/eclipseKepler
	SIRIUS_WORKSPACE="/home/zealot/workspace_sirius"
}

main() {
	setConstant
	setUserConfig

	# process arguments
	while [[ $# -gt 0 ]]
	do
		case "$1" in
			-o)
			DEST_FOLDER="$2"
			if [[ $DEST_FOLDER == "" ]]; then echoerr "-o argument required"; usage; fi
			shift 2
			;;
			--out=*)
			DEST_FOLDER="${1#*=}"
			shift 1
			;;
			-a)
			VER_OFFSET="$2"
			if [[ $VER_OFFSET == "" ]]; then echoerr "-a argument required"; usage; fi
			shift 2
			;;
			--add=*)
			VER_OFFSET="${1#*=}"
			shift 1
			;;
			--from-source)
			FROM_SOURCE="yes"
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

	if [[ "$FROM_SOURCE" == "yes" ]]; then
		# create a temp folder
		TMP_DIR=$(mktemp -d)
		echoinfo "Using $TMP_DIR to build from source"
		cd $TMP_DIR
		SIRIUS_WORKSPACE=$TMP_DIR

		# Clone the source
		echoinfo "Cloning xmsdk"
		git clone git@192.168.9.253:bluefin/xmsdk.git
		echoinfo "Cloning surisdk"
		git clone git@192.168.9.253:bluefin/surisdk.git
		echoinfo "Cloning suribootloader"
		git clone git@192.168.9.253:bluefin/suribootloader.git
	fi

	TIME_START=$(date +%s.%N)
	#
	# Check if offset not zero, if yes, create a output folder name accordingly
	#
	if [[ "${VER_OFFSET}" != "0" ]]; then
		DEST_FOLDER="${DEST_FOLDER}_offset_${VER_OFFSET}"
	fi

	#
	# Check if DEST_FOLDER already exist, if yes, ask user to remove it
	#
	if [ ! -d $DEST_FOLDER ]; then
		echoinfo "Creating $DEST_FOLDER folder"
	else
		echoinfo "$DEST_FOLDER folder already present, do you want to remove it ?"
		check_yn_question "Do you want to remove that existing directory? [y/n]"
		if [ $bResult == "y" ]; then
			rm -rf $DEST_FOLDER
			echoinfo "The $DEST_FOLDER is removed, and re-created"
		else
			echoinfo "The $DEST_FOLDER won't be removed"
		fi
	fi
	echo ""
	mkdir -p $DEST_FOLDER
	mkdir -p $DEST_FOLDER/json
	mkdir -p $DEST_FOLDER/tar_xz
	mkdir -p $DEST_FOLDER/binary

	#
	# Start the build process
	#
	count=${#PROJECTS[@]}
	for ((j=0; j < $count; j++)); do
		project=${PROJECTS[j]}
		config=${BUILD_CONFIGS[j]}
		fwOut=${FIRMWARE_NAME_OUT[j]}
		fwStr=${FIRMWARE_STR[j]}
		fwBinExtOut=${FIRMWARE_BINARY_EXTENSION_OUT[j]}
		fwVerFile="${SIRIUS_WORKSPACE}/${project}/${FIRMWARE_VER_FILE[j]}"

		TIME_FW_START=$(date +%s.%N)
		if [[ "${VER_OFFSET}" != "0" ]]; then
			# avoid $fwStr, because if this argument is empty, the funtion will understand only 1 argc is passed to it
			# use "${fwStr}" instead, the arg will remain
			getFirmwareVersion "${fwStr}" "${fwVerFile}"
			newMajor=$((verMajor+VER_OFFSET))
			newMinor=$((verMinor+VER_OFFSET))
			newRev=$((verRev+VER_OFFSET))
			setFirmwareVersion "${fwStr}" "${fwVerFile}" $newMajor $newMinor $newRev
			echoinfo "Set $fwOut version from $verMajor.$verMinor.$verRev to $newMajor.$newMinor.$newRev"
		else
			getFirmwareVersion "${fwStr}" "${fwVerFile}"
			echoinfo "Building $fwOut version $verMajor.$verMinor.$verRev"
		fi
		buildProject 		$SIRIUS_WORKSPACE $project $config
		getOutputFirmware	$SIRIUS_WORKSPACE $project $config "$fwOut.json" $DEST_FOLDER
		getOutputFirmware	$SIRIUS_WORKSPACE $project $config "$fwOut.json" $DEST_FOLDER/json
		getOutputFirmware	$SIRIUS_WORKSPACE $project $config "$fwOut.json.tar.xz" $DEST_FOLDER
		getOutputFirmware	$SIRIUS_WORKSPACE $project $config "$fwOut.json.tar.xz" $DEST_FOLDER/tar_xz
		getOutputFirmware	$SIRIUS_WORKSPACE $project $config "${fwOut}${fwBinExtOut}" $DEST_FOLDER
		getOutputFirmware	$SIRIUS_WORKSPACE $project $config "${fwOut}${fwBinExtOut}" $DEST_FOLDER/binary
		if [[ "${VER_OFFSET}" != "0" ]]; then
			setFirmwareVersion "${fwStr}" "${fwVerFile}" $verMajor $verMinor $verRev
			echoinfo "Set $fwOut back to old version: $verMajor.$verMinor.$verRev"
		fi
		TIME_FW_END=$(date +%s.%N)
		TIME_FW_DIFF=$(echo "$TIME_FW_END - $TIME_FW_START" | bc)
		echoinfo "Build $fwOut firmware successfully in $TIME_FW_DIFF (s)"
		echo ""
	done

	#
	# Copy the Fixed firmware to $DEST_FOLDER folder
	#
	if [[ -d "./sirius_fixed_firmware" ]]; then
		echoinfo "SIRIUS_FIXED_FIRMWARE folder found, copy the fixed firmware to $DEST_FOLDER"
		cp ./sirius_fixed_firmware/* $DEST_FOLDER
		cp ./sirius_fixed_firmware/*.json $DEST_FOLDER/json
		cp ./sirius_fixed_firmware/*.tar.xz $DEST_FOLDER/tar_xz
		echoinfo "Create the checksum.all.sum file"
		cd $DEST_FOLDER && find *.tar.xz -type f -exec md5sum {} \; | sort -k 2 | md5sum > md5sum.all.sum
		echoinfo "Create the factory folder, this folder is useful for making flasher"
		mkdir factory
		cp *.tar.xz factory
		cp md5sum.all.sum factory
	fi

	#
	# Print the build time
	#
	TIME_END=$(date +%s.%N)
	TIME_DIFF=$(echo "$TIME_END - $TIME_START" | bc)
	echoinfo "Build firmware successfully in $TIME_DIFF (s)"
}

##################################################################################################
# main program
##################################################################################################
main $@
