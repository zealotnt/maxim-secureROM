#!/bin/bash

# Old version - 3PCB --> enable this
# if [ ! -d /sys/class/gpio/gpio81 ]; then
# 	echo "Export GPIO81 to manually control it"
# 	echo 81 > /sys/class/gpio/export
# else
# 	echo "Already exported"
# fi
# echo "Going to pull reset pin low"
# echo low > /sys/class/gpio/gpio81/direction
# sleep 1
# echo "Going to pull reset pin high"
# echo high > /sys/class/gpio/gpio81/direction
# echo "Maxim ready to start over"


# newer version - P1 --> enable this
if [ ! -d /sys/class/gpio/gpio81 ]; then
	echo "Export GPIO81 to manually control it"
	echo 81 > /sys/class/gpio/export
else
	echo "Already exported"
fi
echo "Going to pull reset pin high"
echo high > /sys/class/gpio/gpio81/direction
sleep 1
echo "Going to pull reset pin low"
echo low > /sys/class/gpio/gpio81/direction
echo "Maxim ready to start over"