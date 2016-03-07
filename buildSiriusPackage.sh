#!/bin/bash
. Host/customer_scripts/scripts/colorCode.sh

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


if [ ! -d ../secureROM-Sirius ]; then
	echo "Creating secureROM-Sirius folder"
	cd ..
	mkdir secureROM-Sirius
	cd secureROM-Sirius
	currentDir=$(pwd)
	echo $currentDir
else
	echo "secureROM-Sirius folder Already present, do you want to remove it ?"
	check_yn_question "Do you want to remove that existing directory? [y/n]"
	if [ $bResult == "y" ]; then
		cd ..
		rm -rf secureROM-Sirius
		mkdir secureROM-Sirius
		cd secureROM-Sirius
		currentDir=$(pwd)
	else
		exit 1	
	fi
fi

cp ../secureROM/setup.sh $currentDir/setup.sh
# Make the folder structure of secureROM-Sirius
mkdir Host; cd Host;
mkdir customer_scripts; cd customer_scripts;
mkdir lib;

# Copy all of content in support-scripts folder
check_yn_question "Do you want to copy support-scripts as well ? [y/n]"
if [ $bResult == "y" ]; then
	mkdir ../../support-scripts
	tempDir=$(pwd)
	cd -P ../../../secureROM/support-scripts/
	cp -rp * $currentDir/support-scripts/
	cd "$tempDir"
fi


# Copy all of content in keys folder
cp -rf ../../../secureROM/Host/customer_scripts/keys $currentDir/Host/customer_scripts/keys

# Copy all of content in lib/serial_sender folder
cp -rf ../../../secureROM/Host/customer_scripts/lib/serial_sender $currentDir/Host/customer_scripts/lib/serial_sender
# Except for these files
rm lib/serial_sender/progressbar-2.3.tar.gz
rm lib/serial_sender/pyserial-2.7.tar.gz
rm lib/serial_sender/python27.dll

# Copy some script and package in scripts folder
mkdir scripts; cd scripts
cp ../../../../secureROM/Host/customer_scripts/scripts/colorCode.sh .
cp ../../../../secureROM/Host/customer_scripts/scripts/scp_settings.txt .
cp ../../../../secureROM/Host/customer_scripts/scripts/sendscp_mod.sh .
mkdir buildSCP
cp -rf ../../../../secureROM/Host/customer_scripts/scripts/buildSCP/prod_p3_write_crk ./buildSCP/prod_p3_write_crk
cp -rf ../../../../secureROM/Host/customer_scripts/scripts/buildSCP/testRTC ./buildSCP/testRTC
cp -rf ../../../../secureROM/Host/customer_scripts/scripts/buildSCP/maximFw ./buildSCP/maximFw

echo -e "${ENDL}${KLRED}${KBOLD}Done build secureROM-Sirius!!!${KRESET}"