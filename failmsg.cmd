@echo off
:: 2011/08/10, SUN

if NOT [%2]==[] goto have_errorlevel

echo --ERROR: %1
exit /b 1
goto end

:have_errorlevel
echo --ERROR: %1, ERRORLEVEL %2
exit /b %2

:end
