@echo off
setlocal
title GhostControl Manba V2 NBJr
cd /d "%~dp0"
echo GhostControl Manba V2 NBJr - launcher
echo Choix disponible: KILL+ELF / ELF seulement / KILL seulement
echo.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Launch-GhostControl-ManbaV2-NBJr.ps1"
pause
