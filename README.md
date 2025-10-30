# Construtor de Torres (Windows)

Jogo didático de SO com dois clientes acessando um servidor, compartilhando estado via memória mapeada e exclusão mútua por mutex.

## Compilação (MinGW)

```
gcc -Wall -Wextra -g3 main.c server.c client.c -lws2_32 -o output/jogo.exe
```

## Servidor (exemplos de flags)

```
output\jogo.exe servidor --modo treino --tempo 30000 --sem-risco --sem-sabotagem --max 50 --duracao 120000
```
- --modo treino|normal
- --tempo <ms>
- --sem-risco
- --sem-sabotagem
- --max <N>
- --duracao <ms> (encerra por tempo; decide por pontos; pode dar EMPATE)

## Cliente (exemplos de flags)

```
output\jogo.exe cliente --hist 12
output\jogo.exe cliente --hist 12 --sem-cores
```
- --hist <N>
- --sem-cores

## Controles

- ADD X (1..5)
- SABOTAR Y (1..5) quando evento permitir
- STATUS
- SAIR

## Eventos

- DOUBLE (dobra adição)
- RISK (risco de cair com ADD 4/5)
- SABOTAR (remove blocos)

## Automação (opcional)

```
set TORRE_CMDS=ADD 3,ADD 4,SABOTAR 2
output\jogo.exe cliente
```

## Observações

- Duração: se --duracao for usado, ao acabar o tempo vence quem tiver mais pontos; se empatar, o resultado é EMPATE.
- Tempo padrão por turno: 30s (ajustável por --tempo).

## Tutorial passo a passo (Windows CMD)

### 1) Abrir o Prompt de Comando

- Pressione Win+R, digite `cmd` e aperte Enter.

### 2) Ir até a pasta do projeto

Use `cd` para entrar na pasta. Exemplo:

```
cd C:\Users\SEU_USUARIO\OneDrive\programacao\c\construtor_de_torres
```

### 3) Compilar

Opção A: usando MinGW (recomendado se tiver `gcc` no PATH)

```
build_mingw.bat
```

Opção B: usando MSVC (abra antes o “Developer Command Prompt for VS”)

```
build_msvc.bat
```

Opção C: manual pelo gcc (MinGW)

```
gcc -Wall -Wextra -g3 main.c server.c client.c -lws2_32 -o output/jogo.exe
```

Após compilar, o executável estará em `output\jogo.exe`.

### 4) Executar o servidor

No mesmo CMD (ou um novo):

```
output\jogo.exe servidor --modo treino --tempo 30000 --max 50 --duracao 120000
```

Anotações:
- `--modo treino` desativa RISK e SABOTAR. Use `--modo normal` ou omita para o modo normal.
- `--tempo 30000` define 30 segundos por turno.
- `--max 50` define a altura-alvo da torre.
- `--duracao 120000` encerra a partida após 120s e decide por pontos (pode empatar).

Deixe a janela do servidor aberta.

### 5) Abrir dois clientes (dois terminais)

Abra dois CMDs e, em cada um:

```
cd C:\Users\SEU_USUARIO\OneDrive\programacao\c\construtor_de_torres
output\jogo.exe cliente --hist 12
```

Você pode desativar cores com `--sem-cores` se o terminal não suportar.

### 6) Jogando

- Comandos válidos no cliente:
  - `ADD X` (1..5)
  - `SABOTAR Y` (1..5) quando o evento SABOTAR estiver ativo
  - `STATUS`
  - `SAIR`
- Eventos:
  - DOUBLE: sua adição vale em dobro
  - RISK: usar `ADD 4` ou `5` pode derrubar a torre
  - SABOTAR: remove blocos

Objetivos e fim de jogo:
- Vence quem atingir `MAX` primeiro ou tiver mais pontos ao final de `--duracao`.
- Se os pontos forem iguais ao fim do tempo, o resultado é EMPATE.

### 7) Automação de jogadas (opcional)

Em um CMD de cliente, antes de rodar o cliente:

```
set TORRE_CMDS=ADD 3,ADD 4,SABOTAR 2
output\jogo.exe cliente
```

### 8) Dicas de solução de problemas

- Se o terminal mostrar caracteres estranhos, use ASCII (já padrão) ou rode `chcp 65001` antes do cliente.
- Se o cliente não conectar, verifique firewall/antivírus e se o servidor está rodando.
- Se portas estiverem em uso, feche instâncias antigas e rode novamente. A porta padrão é `5055` (ajuste em `common.h` se necessário e recompile).
