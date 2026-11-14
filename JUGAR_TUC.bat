@echo off
cd /d "C:\Proyectos\TheUnfinishedCut"
git checkout -- Config/DefaultInput.ini >nul 2>&1
start "" "C:\Users\Carlos\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Proyectos\TheUnfinishedCut\TheUnfinishedCut.uproject" /Game/TUC/Maps/L0_Campaign -game -ResX=1600 -ResY=900 -windowed
