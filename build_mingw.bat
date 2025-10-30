@echo off
rem Compila o jogo Construtor de Torres usando MinGW
setlocal

if not exist output mkdir output

gcc -Wall -Wextra -g3 ^
  main.c server.c client.c ^
  -lws2_32 ^
  -o output\jogo.exe

if %ERRORLEVEL% neq 0 (
  echo Erro na compilacao!
  exit /b %ERRORLEVEL%
) else (
  echo Compilado com sucesso: output\jogo.exe
  echo Rode:
  echo   output\jogo.exe servidor
  echo   output\jogo.exe cliente
)
endlocal
