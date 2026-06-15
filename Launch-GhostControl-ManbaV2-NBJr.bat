@echo off
setlocal
title GhostControl Manba V2 NBJr
cd /d "%~dp0"
echo GhostControl Manba V2 NBJr - cleanup puis payload
echo.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Launch-GhostControl-ManbaV2-NBJr.ps1"
pause
