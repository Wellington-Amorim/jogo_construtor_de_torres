@echo off
rem Compila o jogo Construtor de Torres usando MinGW
setlocal

pushd "%~dp0.."
if %ERRORLEVEL% neq 0 (
  echo Nao foi possivel acessar a pasta raiz do projeto.
  set EXIT_CODE=1
  goto wait_prompt
)

if not exist output mkdir output

gcc -Wall -Wextra -g3 ^
  main.c server.c client.c ^
  -lws2_32 ^
  -o output\jogo.exe

if %ERRORLEVEL% neq 0 (
  echo Erro na compilacao!
  set EXIT_CODE=%ERRORLEVEL%
  goto wait_prompt
) else (
  echo Compilado com sucesso: output\jogo.exe
  echo Rode:
  echo   output\jogo.exe servidor
  echo   output\jogo.exe cliente
  set EXIT_CODE=0
)

(goto) 2>nul & rem limpa erro do label antes do pause
:wait_prompt
echo.
echo Pressione qualquer tecla para sair...
pause > nul
popd
endlocal & exit /b %EXIT_CODE%
