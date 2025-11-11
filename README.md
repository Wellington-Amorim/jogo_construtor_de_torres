# Build Up! (Windows)

<img width="700" alt="Imagem do jogo" src="https://github.com/user-attachments/assets/1f726c40-be70-4620-b562-afae9b446a22" />

## Sobre o jogo

O Build Up! nasceu como uma forma de praticar conceitos de Sistemas Operacionais sem ficar só na teoria. Dois jogadores se conectam ao mesmo servidor, cada um tentando empilhar blocos na própria torre enquanto lida com eventos aleatórios que mudam o turno (DOUBLE, RISK, SABOTAR). Os comandos são enviados pelo terminal (`ADD`, `SABOTAR`, `STATUS`, `SAIR`) e o servidor central decide o resultado de cada rodada. Vence quem chegar primeiro na altura alvo ou quem terminar com mais pontos quando o cronômetro estourar, então dá para brincar com estratégia e concorrência ao mesmo tempo.

## Linguagem adotada

Escrevemos tudo em **C** para ter acesso direto às APIs do Windows (Winsock, mutex nomeado, memória compartilhada). A ideia era mostrar, do jeito mais próximo possível das aulas, como processos reais conversam, sincronizam e usam primitivas do SO para dividir recursos.

## Comunicação entre processos

Servidor e clientes trocam duas coisas: mensagens TCP (Winsock) para comandos/notificações e um bloco de memória compartilhada para o estado em tempo real. O servidor envia tokens como `ID`, `VEZ`, `TORRE=…` e mensagens de evento; os clientes respondem com `ADD`, `SABOTAR`, etc. Ao mesmo tempo, o servidor mantém altura, pontos, evento e configurações em um `EstadoTorre` mapeado para todos, e os clientes só leem esse bloco para redesenhar a HUD sem pedir tudo via socket.

## Exclusão mútua

Como só o servidor altera o estado, ele usa um mutex nomeado (`CreateMutexW` + `WaitForSingleObject`/`ReleaseMutex`) para proteger cada atualização de torre, pontuação ou penalidade por tempo. Cada turno passa por essa trava antes de mudar o bloco compartilhado, garantindo que não existam resultados intercalados ou leituras inconsistentes quando os clientes consultam o mesmo estado.

## Visão geral rápida

- Windows + WinSock2 + primitivas de sincronização Win32.
- `server.c` coordena tempo e eventos; `client.c` exibe HUD e aceita comandos.
- `common.h` centraliza a estrutura do bloco compartilhado e flags de configuração.
- Binários saem em `output\jogo.exe`.

## Compilação

- MinGW (fluxo que uso no dia a dia): `scripts\build_mingw.bat`
- MSVC (abrindo o *Developer Command Prompt*): `scripts\build_msvc.bat`
- Manual (mesmo comando do script):
  ```
  gcc -Wall -Wextra -g3 main.c server.c client.c -lws2_32 -o output/jogo.exe
  ```

## Executando

### Servidor

```
output\jogo.exe servidor --modo treino --tempo 30000 --sem-risco --sem-sabotagem --max 50 --duracao 120000
```
- `--modo treino|normal`
- `--tempo <ms>` (30.000 por padrão)
- `--sem-risco`, `--sem-sabotagem` para testar com calma
- `--max <N>` altura desejada
- `--duracao <ms>` encerra por tempo e decide por pontos

### Cliente

```
output\jogo.exe cliente --hist 12
output\jogo.exe cliente --hist 12 --sem-cores
```
- `--hist <N>` define quantas jogadas aparecem no terminal
- `--sem-cores` ajuda em terminais sem suporte a ANSI

## Controles rápidos

- `ADD X` (1..5)
- `SABOTAR Y` (1..5) quando o evento permite
- `STATUS`
- `SAIR`

## Eventos

- **DOUBLE**: dobra os blocos adicionados
- **RISK**: `ADD 4` ou `ADD 5` podem derrubar a torre
- **SABOTAR**: remove blocos do oponente quando acionado

## Automação opcional

```
set TORRE_CMDS=ADD 3,ADD 4,SABOTAR 2
output\jogo.exe cliente
```

## Observações

- Se `--duracao` for usado, ao fim do tempo vence quem tiver mais pontos (empates acontecem).
- Turnos duram 30 s por padrão (`--tempo`).
- Porta padrão: `5055` (mude em `common.h` e recompile se precisar).

## Autores

- Bruno Antonio Miguel Soria Toniolo
- Kauan Marcolino Garcia
- Wellington Amorim de Sousa

## Tutorial passo a passo (Windows CMD)

1. **Abrir o Prompt** — `Win + R`, digite `cmd`, Enter.
2. **Entrar na pasta** — `cd C:\Users\SEU_USUARIO\OneDrive\programacao\c\build_up`
3. **Compilar**
   - MinGW: `scripts\build_mingw.bat`
   - MSVC: `scripts\build_msvc.bat`
   - Manual: `gcc -Wall -Wextra -g3 main.c server.c client.c -lws2_32 -o output/jogo.exe`
4. **Subir o servidor**
   ```
   output\jogo.exe servidor --modo treino --tempo 30000 --max 50 --duracao 120000
   ```
5. **Abrir dois clientes**
   ```
   cd C:\Users\SEU_USUARIO\OneDrive\programacao\c\build_up
   output\jogo.exe cliente --hist 12
   ```
   Use `--sem-cores` se o terminal não gostar de ANSI.
6. **Comandos durante o jogo**
   - `ADD X`, `SABOTAR Y`, `STATUS`, `SAIR`
   - Eventos informados pelo servidor controlam quando é seguro arriscar.
7. **Automatizar testes**
   ```
   set TORRE_CMDS=ADD 3,ADD 4,SABOTAR 2
   output\jogo.exe cliente
   ```
8. **Solução de problemas**
   - Rode `chcp 65001` se aparecerem caracteres estranhos.
   - Verifique firewall/antivírus se o cliente não conectar.
   - Encerre instâncias antigas para liberar a porta 5055.
