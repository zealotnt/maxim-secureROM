#!/bin/bash

case $# in
1)	fileBin=$(basename $1)
	fileName=${fileBin%.bin}
	;;
*)	echo "Usage: ./casignBuild.sh <binfile.bin>"
	exit 1
	;;
esac

echo "Going to translate filename=$fileBin to $fileName.sbin"
sleep 1s

cp -vf $1 ../CaSign/
cd ../CaSign ; ./ca_sign_build algo=ecdsa ca="$fileBin" sca="$fileName.sbin" \
load_address=10000000 \
jump_address=10000020 \
arguments=  \
version=01000003 \
application_version=01010000 \
verbose=yes


# Copy as .sbin file name, for SCP purpose
cp "$fileName.sbin" ../scripts/buildSLA/
# Copy as .bin file name, for JLink fw loading purpose
cp "$fileName.sbin" ../scripts/buildSLA/"binNamed_$fileName.bin"

# Remove gemerated file
rm "$fileName.bin"
rm "$fileName.sbin"
cd ../scripts

echo "Copied $fileName.sbin as /scripts/SLA/$fileName.sbin"
echo "Copied $fileName.sbin as /scripts/SLA/binNamed_$fileName.bin"

exit 0