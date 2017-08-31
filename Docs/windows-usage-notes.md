* 31st August 2017:
    - Try to stress test 043 bootloader firmware loading through USB-SCP
    - Problem raise:
        1. _reconfigure can't work, need to comment out this line of code in pyserial library
            - line 78 of `serialwin32.py`
            - the problem raise at line 222 of `serialwin32.py`

        2. Manual command to run `serial_sender.py` from `cmd`
        ```
        cd .../secureROM
        python Host/customer_scripts/lib/serial_sender/serial_sender.py -s COMxx -t 2 -v -w tempFw/packet.list
        ```
        3. Or run the auto usb on-off test in command from `cmd`
        ```
        cd .../secureROM/test_usb_scp
        python test_usb.py
        ```
