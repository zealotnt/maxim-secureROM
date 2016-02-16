#!/bin/bash

#####################################################################
# Check number of input parameter
#####################################################################
case $# in
2)	readonly inputBinFile=$1
	readonly targetCOMPort=$2
	;;
*)	echo "Unexpected argument"
	echo "Usage: bash doItAll.sh <inputBinFile> <targetCOMPort>"
	exit 1
	;;
esac

#####################################################################
# Check presence of input argument
#####################################################################
if [ ! -f "$inputBinFile" ]; then
	echo "Error: the <inputBinFile> ($inputBinFile) does not exist."
	exit 2
# These code doesn't work yet
# elif [ ! -d "$targetCOMPort" ]; then
# 	echo "Error: the <targetCOMPort> ($targetCOMPort) does not exist."
# 	exit 2
fi


#####################################################################
# Going to build sbin file
#####################################################################
echo "Building sbin file from $inputBinFile"
sleep 1s
./casignBuild.sh $inputBinFile
retVal=$?

if [ $retVal != 0 ]; then
	echo "return value from 'casignBuild.sh'=$retVal "
	echo "Build sbin using 'casignBuild.sh' fail !!!"
	exit 2
fi

inputBinWithoutPath=$(basename $1)
inputFileName=${inputBinWithoutPath%.bin}
#####################################################################
# Going to build SCP packets
#####################################################################
echo "Building SCP packets from $inputBinFile"
sleep 1s
bash ./build_application_mod.sh buildSLA/$inputFileName.sbin $inputFileName
retVal=$?

if [ $retVal != 0 ]; then
	echo "return value from 'build_application_mod.sh'=$retVal "
	echo "Build SCP packet using 'build_application_mod.sh' fail !!!"
	exit 2
fi

#####################################################################
# Going to flash to board
#####################################################################
echo "Flashing to board using SCP"
sleep 1s
bash ./sendscp_mod.sh $targetCOMPort buildSCP/$inputFileName
retVal=$?

if [ $retVal != 0 ]; then
	echo "Can't flash to board !!!"
	exit 2
fi
