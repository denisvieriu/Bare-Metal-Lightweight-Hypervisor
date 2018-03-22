::
:: usage: upload_to_pxe.cmd ProjectName ProjectDir ProjectPlatform ProjectConfig
::
@echo off

setlocal
set prj_name=%1
set prj_dir=%2
set prj_platform=%3
set prj_config=%4

echo %cd%\upload_to_pxe.cmd %prj_name% %prj_dir% %prj_platform% %prj_config%

::
:: --- validate parameters and set internal (final) platform and config ---
::

:: check that none of the parameters is empty
if [%prj_name%]==[] failmsg.cmd "project name not specified (1st arg)"
if [%prj_dir%]==[] failmsg.cmd "project dir not specified (2nd arg)"
if [%prj_platform%]==[] failmsg.cmd "project platform not specified (3rd arg)"
if [%prj_config%]==[] failmsg.cmd "project config not specified (4th arg)"

:: --- validate project directory ---
if NOT EXIST %prj_dir% failmsg.cmd "invalid project dir %prj_dir%"

:: --- validate platform (only X86 and X64 are accepted) ---
if /I %prj_platform%==Win32 goto platform_x86
if /I %prj_platform%==x64 goto platform_x64
failmsg.cmd "invalid platform %prj_platform%"
:platform_x86
set int_platform=x86
set sufix=
goto platform_ok
:platform_x64
set int_platform=x64
set sufix=
goto platform_ok
:platform_ok

:: --- validate config (only USER MODE specifics are accepted) ---
if /I %prj_config%==Debug goto config_debug
if /I %prj_config%==Release goto config_release
failmsg.cmd "invalid config %prj_config%"
:config_debug
set int_config=chk
goto config_ok
:config_release
set int_config=fre
goto config_ok
:config_ok

:: --- save current directory & change to root of this script ---
pushd %cd%
cd %~dp0%

:: --- call CFGDIRS ---
call cfgdirs.cmd
if NOT [%config_dirs_set%]==[1] failmsg.cmd "ERROR, CFGDIRS failed"

if [%PXE_PATH%]==[__EMPTY__] goto no_pxe
if [%PXE_PATH%]==[] goto no_pxe
echo put bin\x64\%prj_config%\minihv.bin %PXE_PATH%/pxeboot.bin  (using PXE_PATH %PXE_PATH%)
:: --- do WINSCP stuff ---
winscp /command "open ftp://tftp:tftp@access-pxe.clj.bitdefender.biz -passive=on" "option confirm off" "put bin\x64\%prj_config%\minihv.bin %PXE_PATH%/pxeboot.bin" "exit"
:no_pxe

if [%PXE_PATH2%]==[__EMPTY__] goto no_pxe_2
if [%PXE_PATH2%]==[] goto no_pxe_2
echo put bin\x64\%prj_config%\minihv.bin %PXE_PATH2%/pxeboot.bin  (using PXE_PATH2 %PXE_PATH2%)
:: --- do WINSCP stuff ---
winscp /command "open ftp://tftp:tftp@access-pxe.clj.bitdefender.biz -passive=on" "option confirm off" "put bin\x64\%prj_config%\minihv.bin %PXE_PATH2%/pxeboot.bin" "exit"
:no_pxe_2

:: Wait 3000
copy C:\Users\dvieriu\Desktop\[DEL]Shortcoming\minihv\bin\x64\Debug\minihv.bin c:\VM\tftpfolder\

:: --- reload initial current directory ---
popd
