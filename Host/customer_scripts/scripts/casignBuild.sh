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

cp "$fileName.sbin" ../scripts/buildSLA/

cd ../scripts

echo "Copied $fileName.sbin to /scripts/SLA/"

exit 0