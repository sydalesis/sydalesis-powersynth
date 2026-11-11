@echo off
rem Sydalesis 6.0.0 installer (formerly Driftfield). Installs Sydalesis.vst3 / .clap as a NEW plug-in next to
rem Driftfield, which stays installed for sessions made with it. Run as administrator (Common Files is protected).
setlocal
set "HERE=%~dp0"

echo Installing VST3...
xcopy /E /I /Y "%HERE%Sydalesis.vst3" "%CommonProgramFiles%\VST3\Sydalesis.vst3" >nul || goto :fail
echo Installing CLAP...
if not exist "%CommonProgramFiles%\CLAP" mkdir "%CommonProgramFiles%\CLAP"
copy /Y "%HERE%Sydalesis.clap" "%CommonProgramFiles%\CLAP\Sydalesis.clap" >nul || goto :fail

for /f "usebackq delims=" %%D in (`powershell -NoProfile -Command "[Environment]::GetFolderPath('MyDocuments')"`) do set "DOCS=%%D"
echo Installing wavetable library to "%DOCS%\Sydalesis\Wavetables"...
xcopy /E /I /Y "%HERE%Wavetables" "%DOCS%\Sydalesis\Wavetables" >nul || goto :fail
echo Installing grain samples to "%DOCS%\Sydalesis\Samples"...
xcopy /E /I /Y "%HERE%Samples" "%DOCS%\Sydalesis\Samples" >nul || goto :fail
echo Installing tunings to "%DOCS%\Sydalesis\Tunings"...
xcopy /E /I /Y "%HERE%Tunings" "%DOCS%\Sydalesis\Tunings" >nul || goto :fail

echo.
echo Done. Rescan plug-ins in Studio One: Sydalesis appears as a new instrument (vendor Sydalesis).
echo The first time it opens, your Driftfield presets, favourites and captures are copied into Documents\Sydalesis.
echo Driftfield is left installed and untouched.
pause
exit /b 0

:fail
echo.
echo Something failed. Make sure you ran this as administrator.
pause
exit /b 1
