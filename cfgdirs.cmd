@echo off

call :seterrorlevel 0

::
:: --- check if CFGDIRS was already executed (to avoid running it multiple times) ---
::
if NOT X%config_dirs_set%X==XX echo CFGDIRS called, but settings already present...
if NOT X%config_dirs_set%X==XX goto :eof

::
:: --- select configuration based on local host ---
::
if _%COMPUTERNAME%_==_IANICHITEI-LPT_ goto config_IANICHITEI
if _%COMPUTERNAME%_==_DVIERIU-LPT_ goto config_dvieriu

:: --- unrecognized host ==> ERROR ---
echo.
echo ERROR: host '%COMPUTERNAME%' not recognized by CFGDIRS.CMD, build FAILED!
echo You need to add a host dedicated section to CFGDIRS.CMD and retry build.
echo.
goto end



:: -----------------------------------------------------------------------------
:: IANICHITEI
:: -----------------------------------------------------------------------------

:config_IANICHITEI

::
:: --- set the ROOT directory for the build process (current directory, where CFGDIRS resides)
::
set rootdir=%~dp0%


::
:: --- conventional directory junctions based settings ---
::

:: --- WDK root directory ---
set wdkbase=C:\DDK.LATEST

:: --- Platform SDK root directory ---
set sdkbase=C:\SDK.LATEST

:: --- WinDBG root directory ---
set dbgbase=C:\DBG.LATEST

:: --- Visual Studio root directory (likely VS 2010) ---
set vsbase=C:\VS.LATEST

:: --- SVN local root directory (OPTIONAL, leave __EMPTY__ if SVN not needed) ---
set svnroot=__EMPTY__

:: --- PXE on-server path
:::set PXE_PATH=pxe-sandor
:::set PXE_PATH2=pxe-jtag1
set PXE_PATH=__EMPTY__
set PXE_PATH2=pxe-iic

:: --- symbol server - used only for full SDK builds (OPTIONAL) ---
set sym_server=__EMPTY__

:: --- local symbol storage directory - can be used for each build (OPTIONAL) ---
set sym_local=C:\SYMBOLS.TEMP


::
:: --- additional shortcut settings ---
::
set bscmake_exe="%VSBASE%\vc\bin\bscmake.exe"
set devenv_exe="%VSBASE%\Common7\IDE\devenv.exe"
set devenv_2008_exe="c:\program files (x86)\microsoft visual studio 9.0\Common7\IDE\devenv.exe"
set symstore_exe="%DBGBASE%\symstore.exe"
set signtool_exe="%sign_dir%\signtool.exe"

set devenv_com="%VSBASE%\Common7\IDE\devenv.com"
set devenv_2008_com="c:\program files (x86)\microsoft visual studio 9.0\Common7\IDE\devenv.com"

:: end of configuration settings, jump to validation
goto :validate_config

:: -----------------------------------------------------------------------------
:: dvieriu
:: -----------------------------------------------------------------------------

:config_dvieriu

::
:: --- set the ROOT directory for the build process (current directory, where CFGDIRS resides)
::
set rootdir=%~dp0%


::
:: --- conventional directory junctions based settings ---
::

:: --- WDK root directory ---
set wdkbase=C:\DDK.LATEST

:: --- Platform SDK root directory ---
set sdkbase=C:\SDK.LATEST

:: --- WinDBG root directory ---
set dbgbase=C:\DBG.LATEST

:: --- Visual Studio root directory (likely VS 2010) ---
set vsbase=C:\VS.LATEST

:: --- SVN local root directory (OPTIONAL, leave __EMPTY__ if SVN not needed) ---
set svnroot=__EMPTY__

:: --- PXE on-server path
:::set PXE_PATH=pxe-sandor
:::set PXE_PATH2=pxe-jtag1
set PXE_PATH=__EMPTY__
set PXE_PATH2=pxe-dvieriu

:: --- symbol server - used only for full SDK builds (OPTIONAL) ---
set sym_server=__EMPTY__

:: --- local symbol storage directory - can be used for each build (OPTIONAL) ---
set sym_local=C:\SYMBOLS.TEMP


::
:: --- additional shortcut settings ---
::
set bscmake_exe="%VSBASE%\vc\bin\bscmake.exe"
set devenv_exe="%VSBASE%\Common7\IDE\devenv.exe"
set devenv_2008_exe="c:\program files (x86)\microsoft visual studio 9.0\Common7\IDE\devenv.exe"
set symstore_exe="%DBGBASE%\symstore.exe"
set signtool_exe="%sign_dir%\signtool.exe"

set devenv_com="%VSBASE%\Common7\IDE\devenv.com"
set devenv_2008_com="c:\program files (x86)\microsoft visual studio 9.0\Common7\IDE\devenv.com"

:: end of configuration settings, jump to validation
goto :validate_config



::
:: --- validate configurations ---
::
:validate_config

:: --- check WDK / VS / DBG / SDK by their main executables ---


:: --- this config is OK, mark it so ---
echo CFGDIRS successfully set environment variables...
set config_dirs_set=1

:: output also settings
echo WDKBASE %WDKBASE%
echo SDKBASE %SDKBASE%
echo DBGBASE %DBGBASE%
echo VSBASE %VSBASE%
echo BSCMAKE.EXE %bscmake_exe%
echo DEVENV.EXE %devenv_exe%
echo SYMSTORE.EXE %symstore_exe%
echo PXE_PATH %PXE_PATH%
echo PXE_PATH2 %PXE_PATH2%

goto end


::
:: seterrorlevel routine
::
:seterrorlevel
exit /b %1


:end
exit /b 0



