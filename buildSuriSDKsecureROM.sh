#!/bin/bash

# This script will create a folder call /opt/sirius/xmsdk/tools/secureROM/Host/customer_scripts/scripts/
# contain 2 file: "casignBuild_otp.sh" and "buildSCP_otp.sh"
# that can be use witl "mlsBuildSCP.sh"
# NOTE: should run as root user
# add this to /home/zealot/.bashrc
# alias maximBuild="bash /opt/sirius/xmsdk/tools/secureROM/mlsBuildSCP.sh $1"

currentDir=$(pwd)

cd /opt/

if [ ! -d sirius ]; then
	echo "not found sirius folder, create one"
	mkdir sirius
fi

cd sirius

if [ ! -d xmsdk ]; then
	echo "not found xmsdk folder, create one"
	mkdir xmsdk
fi

cd xmsdk

if [ ! -d tools ]; then
	echo "not found tool folder, create one"
	mkdir tools
fi

cd tools

if [ ! -d secureROM ]; then
	echo "not found secureROM folder, create one"
	mkdir secureROM
fi

cd secureROM

cp $currentDir/mlsBuildSCP.sh mlsBuildSCP.sh

if [ ! -d Host ]; then
	echo "not found secureROM folder, create one"
	mkdir Host
fi

cd Host

if [ ! -d customer_scripts ]; then
	echo "not found customer_scripts folder, create one"
	mkdir customer_scripts
fi

cd customer_scripts

if [ ! -d CaSign ]; then
	echo "not found CaSign folder, copy one"
	cp -rp $currentDir/Host/customer_scripts/CaSign CaSign
fi

if [ ! -d keys ]; then
	echo "not found keys folder, copy one"
	cp -rp $currentDir/Host/customer_scripts/keys keys
fi

if [ ! -d lib ]; then
	echo "not found lib folder, copy one"
	cp -rp $currentDir/Host/customer_scripts/lib lib
fi

if [ ! -d scripts ]; then
	echo "not found tool folder, create one"
	mkdir scripts
fi

cd scripts

if [ ! -d buildSLA ]; then
	echo "not found buildSLA folder, create one"
	mkdir buildSLA
fi

if [ ! -d buildSCP ]; then
	echo "not found buildSCP folder, create one"
	mkdir buildSCP
fi

echo "copy buildSCP_otp.sh and casignBuild_otp.sh to its place"
cp $currentDir/Host/customer_scripts/scripts/buildSCP_otp.sh buildSCP_otp.sh
cp $currentDir/Host/customer_scripts/scripts/casignBuild_otp.sh casignBuild_otp.sh

chmod -R 777 /opt/sirius/