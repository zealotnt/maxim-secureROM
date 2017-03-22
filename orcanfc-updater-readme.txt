# How to use
1. Change directory to folder contain the `orcanfc-updater.sh`:
```
cd /home/<user-name>/orcanfc-fw-updater
```

2. Flash the firmware to board:
```
bash orcanfc-updater.sh -p <serial-port> -t ALL -f <path-to-tar.gz-firmware-file>

Example:
***In case:
	+ <serial-port>							:is '/dev/ttyUSB0'
	+ <path-to-tar.gz-firmware-file>		:is 'orcanfc_V4.0.1_pro_signed_by_nets_key.tar.gz'

***The command would be:
bash orcanfc-updater.sh -p /dev/ttyUSB0 -t ALL -f orcanfc_V4.0.1_pro_signed_by_nets_key.tar.gz
```
