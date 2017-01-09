REM call CommonAutomaticCopy.bat

REM @echo on

echo ///////////
echo // DEBUG //
echo ///////////

set src_folder=%1
set dest_folder=%2
set file_name=%3


echo src_folder: %src_folder%
echo dest_folder: %dest_folder%
echo file_name: %file_name%
REM @echo off

REM if %file_name% empty goto exit

REM if not exist %src_folder% goto exit
REM if not exist %dest_folder% goto exit

attrib -r %dest_folder%\%file_name%.exe

copy %src_folder%\%file_name%.exe	%dest_folder%\

attrib +r %dest_folder%\%file_name%.exe

echo Last file copy made at %DATE% %TIME% > LastDebugFileCopy.log


