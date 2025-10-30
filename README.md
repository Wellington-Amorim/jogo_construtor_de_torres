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
