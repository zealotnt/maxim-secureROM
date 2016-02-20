#!/bin/bash

currentDir=$(pwd)

#####################################################################
# Setup environment
#####################################################################
cd /home/root/secureROM-Sirius
bash setup.sh linux ./


#####################################################################
# Going to loadkey to Maxim
#####################################################################
cd /home/root/secureROM-Sirius/Host/customer_scripts/scripts/

bash ./sendscp_mod.sh /dev/ttymxc3 buildSCP/prod_p3_write_crk/ y
retVal=$?

if [ $retVal != 0 ]; then
	echo "Can't loadkey to board !!!"
	exit 2
else 
	echo "Loadkey successfully !!!"
fi

#####################################################################
# Going to load App to Maxim
#####################################################################
bash ./sendscp_mod.sh /dev/ttymxc3 buildSCP/maximFw/ y
retVal=$?

if [ $retVal != 0 ]; then
	echo "Can't flash firmware to board !!!"
	exit 2
else 
	echo "Firmware flash successfully !!!"
fi

exit 0