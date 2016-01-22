#!/bin/bash
echo "Setup Serial for UART1 on Maxim <--> UART2 on A9 <--> ttymxc1"
stty -F /dev/ttymxc1 speed 115200 cs8 -cstopb -parenb
echo "Setup Serial for UART0 on Maxim <--> UART5 on A9 <--> ttymxc4"
stty -F /dev/ttymxc4 speed 115200 cs8 -cstopb -parenb
echo "Setup Serial connection to Host <--> UART4 on A9 <--> ttymxc3"
stty -F /dev/ttymxc3 speed 115200 cs8 -cstopb -parenb
