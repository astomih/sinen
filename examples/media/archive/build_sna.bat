@echo off
setlocal
cd /d "%~dp0"
if exist archive_sample.zip del archive_sample.zip
if exist archive_sample.sna del archive_sample.sna
powershell -NoProfile -ExecutionPolicy Bypass -Command "Compress-Archive -Path sna -DestinationPath archive_sample.zip -Force"
if errorlevel 1 exit /b %errorlevel%
ren archive_sample.zip archive_sample.sna
echo Created archive_sample.sna
