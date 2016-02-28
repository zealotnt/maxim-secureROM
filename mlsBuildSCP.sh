#!/bin/bash

# @brief:
# example: bash mlsBuildSCP.sh surisdk.bin
# --> param[in] = surisdk.bin
# --> return    = surisdk.tar.gz (contain SCP packet of that firmware)

# This script will receive input 1 binary file (Maxim firmware)
# then it will call script store in /opt/sirius/xmsdk/tools/secureROM/Host/customer_scripts/scripts/
# they are "casignBuild_otp.sh" and "buildSCP_otp.sh"
# and output a tar.gz file, that contains SCP packet

currentDir=$(pwd)
#####################################################################
# Check number of input parameter
#####################################################################
case $# in
1)	readonly inputBinFile=$1
	;;
*)	echo "Unexpected argument"
	echo "Usage: bash mlsBuildSCP.sh <inputBinFile>"
	exit 1
	;;
esac


#####################################################################
# Check presence of input argument
#####################################################################
if [ ! -f "$inputBinFile" ]; then
	echo "Error: the <inputBinFile> ($inputBinFile) does not exist."
	exit 2
fi


#####################################################################
# Going to build sbin file
#####################################################################
echo "Building sbin file from $inputBinFile"
sleep 1s
bash /opt/sirius/xmsdk/tools/secureROM/Host/customer_scripts/scripts/casignBuild_otp.sh $inputBinFile $currentDir
retVal=$?

if [ $retVal != 0 ]; then
	echo "return value from 'casignBuild_otp.sh'=$retVal "
	echo "Build sbin using 'casignBuild_otp.sh' fail !!!"
	exit 2
fi

inputBinWithoutPath=$(basename $1)
inputFileName=${inputBinWithoutPath%.bin}

#####################################################################
# Going to build SCP packets
#####################################################################
echo "Building SCP packets from $inputBinFile"
sleep 1s
bash /opt/sirius/xmsdk/tools/secureROM/Host/customer_scripts/scripts/buildSCP_otp.sh $inputFileName.sbin $inputFileName $currentDir
retVal=$?

if [ $retVal != 0 ]; then
	echo "return value from 'buildSCP_otp.sh'=$retVal "
	echo "Build SCP packet using 'buildSCP_otp.sh' fail !!!"
	exit 2
fi

echo "Build success !!!"