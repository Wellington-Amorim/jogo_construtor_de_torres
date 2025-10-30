#ifndef COMMON_H
#define COMMON_H

#ifndef _WIN32
#error Este projeto foi reescrito para Windows (Winsock/Win32). Compile no Windows.
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORTA 5055
#define MAX_ALTURA 50
#define MAX_CLIENTES 2
#define BUF 256

#define MUTEX_NAME      L"Local\\torre_mutex"
#define MAP_NAME        L"Local\\torre_shared"

typedef struct EstadoTorre {
    int altura;
    int vez;
    int max_altura;
    int conectado[MAX_CLIENTES+1];
    int pontos[MAX_CLIENTES+1];
    int evento_atual;
    int tempo_turno_ms;
} EstadoTorre;

static inline void die_last_error(const char* ctx) {
    DWORD e = GetLastError();
    fprintf(stderr, "%s failed. GetLastError=%lu\n", ctx, (unsigned long)e);
    exit(1);
}

static inline void die_wsa(const char* ctx) {
    int e = WSAGetLastError();
    fprintf(stderr, "%s failed. WSA=%d\n", ctx, e);
    exit(1);
}

#endif // COMMON_H
