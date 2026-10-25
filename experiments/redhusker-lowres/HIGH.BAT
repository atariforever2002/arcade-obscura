@echo off
if not exist C:\MAME\RHLOW\MAME52.EXE goto missing
C:
cd \MAME\RHLOW
if not exist MAME52.EXE goto missing
if not exist CWSDPMI.EXE if exist C:\MAME\CWSDPMI.EXE copy C:\MAME\CWSDPMI.EXE CWSDPMI.EXE > NUL
MAME52.EXE rygar -resolution 640x480 -log
if exist ERROR.LOG copy ERROR.LOG HIGH.LOG > NUL
echo.
echo Requested 640x480. Test sound is OFF.
echo Video mode attempts and selected driver:
if exist ERROR.LOG find "Trying" ERROR.LOG
if exist ERROR.LOG find "Found matching" ERROR.LOG
echo Keep this screen for Nora.
goto end
:missing
echo Missing C:\MAME\RHLOW\MAME52.EXE
:end
