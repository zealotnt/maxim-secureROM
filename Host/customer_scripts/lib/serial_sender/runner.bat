set /a SUCCESS=0
set /a FAIL=0
set /a COUNT=0
:while1
rem python serial_sender.py -s COM104 -v -v -f 100 --auto-reset-uart-rtsdts 20180104_orcanfc_fac_D4.1.5\tempFw\packet.list > out_%COUNT%.txt 2>&1
python serial_sender.py -s COM104 -v -v -f 100 --auto-reset-uart-rtsdts 20180104_orcanfc_fac_D4.1.5\tempFw\packet.list

set /a COUNT+=1
if %ERRORLEVEL% == 0 (
	echo "Success!!!"
	set /a SUCCESS+=1
) else (
	echo "Fail!!!"
	set /a FAIL+=1
)
echo "STAT:"
echo "SUCCESS" %SUCCESS%
echo "FAIL" %FAIL%

goto :while1