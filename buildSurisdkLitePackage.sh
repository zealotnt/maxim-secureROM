#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. $DIR/Host/customer_scripts/scripts/colorCode.sh
TARGET_FOLDER=surisdk-lite-fw-updater

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

if [ ! -d ../$TARGET_FOLDER ]; then
	echo "Creating $TARGET_FOLDER folder"
else
	echo "$TARGET_FOLDER folder Already present, do you want to remove it ?"
	check_yn_question "Do you want to remove that existing directory? [y/n]"
	if [ $bResult == "y" ]; then
		rm -rf $TARGET_FOLDER
	fi
fi

cd ..
mkdir $TARGET_FOLDER

# Make the folder structure of secureROM
cd $TARGET_FOLDER
cp $DIR/surisdklite-updater.sh .

mkdir -p Host/customer_scripts/keys
cp -rp $DIR/Host/customer_scripts/keys Host/customer_scripts

mkdir -p Host/customer_scripts/lib/serial_sender
cp -rp $DIR/Host/customer_scripts/lib/serial_sender Host/customer_scripts/lib

mkdir -p Host/customer_scripts/lib/rom
cp -rp $DIR/Host/customer_scripts/lib/rom Host/customer_scripts/lib

mkdir -p Host/customer_scripts/scripts/buildSCP
cp -rp $DIR/Host/customer_scripts/scripts/colorCode.sh Host/customer_scripts/scripts/colorCode.sh
cp -rp $DIR/Host/customer_scripts/scripts/buildSCP/eraser Host/customer_scripts/scripts/buildSCP
cp -rp $DIR/Host/customer_scripts/scripts/buildSCP/OTP_UART_250ms Host/customer_scripts/scripts/buildSCP
cp -rp $DIR/Host/customer_scripts/scripts/buildSCP/prod_p3_write_crk Host/customer_scripts/scripts/buildSCP
cp -rp $DIR/Host/customer_scripts/scripts/buildSCP/suribl-lite Host/customer_scripts/scripts/buildSCP

mkdir -p Host/surisdk-fw-upgrade
cp -rp $DIR/Host/surisdk-fw-upgrade/surisdk_lite_updater Host/surisdk-fw-upgrade/surisdk_lite_updater

echo "Create $TARGET_FOLDER successfully"
