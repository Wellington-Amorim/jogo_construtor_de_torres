#include "common.h"

int servidor_main(int argc, char** argv);
int cliente_main(int argc, char** argv);

int main(int argc, char** argv) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) die_wsa("WSAStartup");

    int rc = 0;
    if (argc < 2) {
        fprintf(stderr, "Uso: %s servidor|cliente\n", argv[0]);
        rc = 1;
    } else if (strcmp(argv[1], "servidor") == 0) {
        rc = servidor_main(argc, argv);
    } else if (strcmp(argv[1], "cliente") == 0) {
        rc = cliente_main(argc, argv);
    } else {
        fprintf(stderr, "Modo invalido: %s. Use 'servidor' ou 'cliente'.\n", argv[1]);
        rc = 1;
    }

    WSACleanup();
    return rc;
}