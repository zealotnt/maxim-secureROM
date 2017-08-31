#!/bin/bash

count=1
while [[ true ]]; do
	read -p "Replug the board, then press enter? [enter]" bResult
	bash orcanfc-updater.sh -p /dev/ttyACM0 -f /home/zealot/tmp/orca_usb/secondBootloadersigned.tar
	echo "Flash success $count times"
	count=$(($count+1))
done
