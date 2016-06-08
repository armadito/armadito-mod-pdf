@echo off

set ANALYZER="a6oPDFAnalyzer.exe"
set DIRPATH=%1

if [%1]==[] goto :help

REM for /R in %DIRPATH%\ %%A do echo "entry = %%A"
REM - FOR /R [[drive:]path] %%parameter IN (set) DO command
FOR /R %DIRPATH% %%E IN (*) DO echo %%E && %ANALYZER% %%E >> result.txt
	
goto :end

:help
echo syntax: scandir.bat [directoryPath]

:end
echo - Exiting...