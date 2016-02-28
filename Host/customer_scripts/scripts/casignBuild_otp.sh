#!/bin/bash

check_yn_question()
{
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

case $# in
2)	fileBin=$(basename $1)
	fileName=${fileBin%.bin}
	readonly outputDir=$2
	;;
*)	echo "Usage: ./casignBuild.sh <binfile.bin>"
	exit 1
	;;
esac

echo "Going to translate filename=$fileBin to $fileName.sbin"
sleep 1s

cp -vf $1 /opt/sirius/xmsdk/tools/secureROM/Host/customer_scripts/CaSign/
cd /opt/sirius/xmsdk/tools/secureROM/Host/customer_scripts/CaSign
 
./ca_sign_build algo=ecdsa ca="$fileBin" sca="$fileName.sbin" \
	load_address=10000000 \
	jump_address=10000020 \
	arguments=  \
	version=01000003 \
	application_version=01010000 \
	verbose=yes

# Copy as .sbin file name, for SCP purpose
cp "$fileName.sbin" ../scripts/buildSLA/

# Copy file .sbin to user's space (not so useful)
# check_yn_question "Do you want to put the $fileName.sbin file to current folder ? [y/n]"
# if [ $bResult == "y" ]; then
# 	cp "$fileName.sbin" $outputDir
# 	echo "$fileName.sbin file created"
# 	sleep 1
# fi

# Copy file fwSigned.bin to user's space 
fileNameSigned=$fileName"Signed"
check_yn_question "Do you want to put the $fileNameSigned.bin file to current folder ? [y/n]"
if [ $bResult == "y" ]; then
	# Copy as .bin file name, for JLink fw loading purpose
	cp "$fileName.sbin" "$fileNameSigned.bin"
	cp "$fileNameSigned.bin" $outputDir
	echo "$fileNameSigned.bin file created"
	sleep 1
fi

# Remove gemerated file
rm "$fileName.bin"
rm "$fileName.sbin"
cd ../scripts

echo "Copied $fileName.sbin as /scripts/SLA/$fileName.sbin"

exit 0