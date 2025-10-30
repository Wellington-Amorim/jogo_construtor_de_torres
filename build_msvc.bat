@echo off
setlocal ENABLEDELAYEDEXPANSION
where cl >nul 2>&1
if errorlevel 1 (
  echo MSVC (cl.exe) nao encontrado. Abra o "Developer Command Prompt for VS".
  exit /b 1
)
cl /nologo /W3 /Zi /EHsc /D_CRT_SECURE_NO_WARNINGS /D_WIN32_WINNT=0x0600 main.c server.c client.c ws2_32.lib /Fe:jogo.exe
if errorlevel 1 exit /b 1
echo Compilado: jogo.exe
