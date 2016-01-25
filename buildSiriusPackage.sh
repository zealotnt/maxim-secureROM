#!/bin/bash

if [ ! -d ../secureROM-Sirius ]; then
	echo "Creating secureROM-Sirius folder"
	cd ..
	mkdir secureROM-Sirius
	cd secureROM-Sirius
	currentDir=$(pwd)
	echo $currentDir
else
	echo "secureROM-Sirius folder Already present, please backup it before run this script !"
	exit 1
fi

cp ../secureROM/setup.sh $currentDir/setup.sh
# Make the folder structure of secureROM-Sirius
mkdir Host; cd Host
mkdir customer_scripts; cd customer_scripts

# Copy all of content in keys folder
cp -rf ../../../secureROM/Host/customer_scripts/keys $currentDir/Host/customer_scripts/keys

# Copy all of content in lib folder
cp -rf ../../../secureROM/Host/customer_scripts/lib $currentDir/Host/customer_scripts/lib
# Except for these files
rm lib/libucl.dll
rm lib/libucl.so.2.4.1
rm lib/session_build.exe
rm lib/serial_sender/MSVCR71.dll
rm lib/serial_sender/library.zip
rm lib/serial_sender/w9xpopen.exe
rm lib/serial_sender/serial_sender.exe

# Copy some script and package in scripts folder
mkdir scripts; cd scripts
cp ../../../../secureROM/Host/customer_scripts/scripts/colorCode.sh .
cp ../../../../secureROM/Host/customer_scripts/scripts/scp_settings.txt .
cp ../../../../secureROM/Host/customer_scripts/scripts/sendscp_mod.sh .
mkdir buildSCP
cp -rf ../../../../secureROM/Host/customer_scripts/scripts/buildSCP/prod_p3_write_crk ./buildSCP/prod_p3_write_crk
