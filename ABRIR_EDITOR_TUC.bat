@echo off
cd /d "C:\Proyectos\TheUnfinishedCut"
git checkout -- Config/DefaultInput.ini >nul 2>&1
start "" "C:\Users\Carlos\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Proyectos\TheUnfinishedCut\TheUnfinishedCut.uproject"
