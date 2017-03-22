#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. $DIR/Host/customer_scripts/scripts/colorCode.sh
TARGET_FOLDER=orcanfc-fw-updater

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

if [ ! -d $DIR/../$TARGET_FOLDER ]; then
	echo "Creating $TARGET_FOLDER folder"
else
	echo "$TARGET_FOLDER folder Already present, do you want to remove it ?"
	check_yn_question "Do you want to remove that existing directory? [y/n]"
	if [ $bResult == "y" ]; then
		rm -rf $DIR/../$TARGET_FOLDER
	fi
fi

cd ..
mkdir $TARGET_FOLDER

# Make the folder structure of secureROM
cd $TARGET_FOLDER
cp 			$DIR/orcanfc-updater.sh 										.
cp 			$DIR/orcanfc-updater-readme.txt 								.

mkdir -p 	Host/customer_scripts/keys
cp -rp 		$DIR/Host/customer_scripts/keys									Host/customer_scripts

mkdir -p 	Host/customer_scripts/lib/serial_sender
cp -rp 		$DIR/Host/customer_scripts/lib/serial_sender					Host/customer_scripts/lib

mkdir -p 	Host/customer_scripts/lib/rom
cp -rp 		$DIR/Host/customer_scripts/lib/rom								Host/customer_scripts/lib

mkdir -p 	Host/customer_scripts/scripts/buildSCP
cp -rp 		$DIR/Host/customer_scripts/scripts/colorCode.sh					Host/customer_scripts/scripts/colorCode.sh
cp -rp 		$DIR/Host/customer_scripts/scripts/buildSCP/nets_key			Host/customer_scripts/scripts/buildSCP

echo "Create $TARGET_FOLDER successfully"
