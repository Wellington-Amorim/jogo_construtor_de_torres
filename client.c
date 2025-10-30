#include "common.h"

static int g_cores_ativo = 0;

static void habilitar_vt(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return;
    mode |= 0x0004;
    if (SetConsoleMode(hOut, mode)) g_cores_ativo = 1;
}

static const char* C_RESET(void){ return g_cores_ativo?"\x1b[0m":""; }
static const char* C_TITLE(void){ return g_cores_ativo?"\x1b[1;36m":""; }
static const char* C_LABEL(void){ return g_cores_ativo?"\x1b[1;37m":""; }
static const char* C_VAL(void){ return g_cores_ativo?"\x1b[1;33m":""; }
static const char* C_OK(void){ return g_cores_ativo?"\x1b[1;32m":""; }
static const char* C_WARN(void){ return g_cores_ativo?"\x1b[1;33m":""; }
static const char* C_ERR(void){ return g_cores_ativo?"\x1b[1;31m":""; }
static const char* C_EVT_DOUBLE(void){ return g_cores_ativo?"\x1b[1;35m":""; }
static const char* C_EVT_RISK(void){ return g_cores_ativo?"\x1b[1;31m":""; }
static const char* C_EVT_SABO(void){ return g_cores_ativo?"\x1b[1;34m":""; }

static void ui_limpar(void) {
    system("cls");
}

static void ui_desenhar_torre(int altura, int max_altura, int meu_id, int vez, int evento, int p1, int p2, int tempo_ms) {
    ui_limpar();
    printf("%s================= CONSTRUTOR DE TORRES =================%s\n", C_TITLE(), C_RESET());
    printf("%sObjetivo:%s %s%d%s blocos\n", C_LABEL(), C_RESET(), C_VAL(), max_altura, C_RESET());
    printf("%sPlacar:%s P1=%s%d%s  P2=%s%d%s\n", C_LABEL(), C_RESET(), C_VAL(), p1, C_RESET(), C_VAL(), p2, C_RESET());
    printf("%sJogador:%s %s%d%s   %sVez:%s %s%d%s\n", C_LABEL(), C_RESET(), C_VAL(), meu_id, C_RESET(), C_LABEL(), C_RESET(), C_VAL(), vez, C_RESET());
    const char* enome = "Nenhum";
    if (evento == 1) enome = "DOUBLE (adicao em dobro)";
    else if (evento == 2) enome = "RISK (alto risco se ADD>=4)";
    else if (evento == 3) enome = "SABOTAR disponivel";
    const char* eclr = (evento==1?C_EVT_DOUBLE():evento==2?C_EVT_RISK():evento==3?C_EVT_SABO():C_LABEL());
    printf("%sEvento:%s %s%s%s\n", C_LABEL(), C_RESET(), eclr, enome, C_RESET());
    if (tempo_ms > 0) printf("%sTempo por turno:%s %s%.1fs%s\n", C_LABEL(), C_RESET(), C_VAL(), tempo_ms/1000.0, C_RESET());
    printf("\nTorre atual (%d/%d):\n\n", altura, max_altura);
    int linhas = max_altura > 25 ? 25 : max_altura;
    double ratio = (max_altura > 0) ? ((double)altura / (double)max_altura) : 0.0;
    int blocos = (int)(ratio * linhas + 0.5);
    for (int i = linhas; i >= 1; --i) {
        if (i <= blocos) {
            printf("|####################|\n");
        } else {
            printf("|                    |\n");
        }
    }
    printf("+--------------------+\n\n");
    printf("%sComandos:%s ADD X (1..5)  |  STATUS  |  SAIR", C_LABEL(), C_RESET());
    if (evento == 3) printf("  |  SABOTAR Y (1..5)");
    printf("\n\n");
    if (evento == 1) printf("Dica: use ADD 4 ou 5 para maximizar o DOUBLE!\n");
    if (evento == 2) printf("Dica: evite ADD 4/5 se nao quer arriscar cair.\n");
    if (evento == 3) printf("Dica: SABOTAR Y remove blocos do total (1..5).\n");
}

static int g_hist_cap = 0;
static char g_historico[50][BUF];
static int g_hist_len = 0;

static void hist_add(const char* s) {
    if (!g_hist_cap) return;
    if (g_hist_len < g_hist_cap) {
        strncpy(g_historico[g_hist_len], s, BUF-1);
        g_historico[g_hist_len][BUF-1] = '\0';
        g_hist_len++;
    } else {
        for (int i=1;i<g_hist_cap;i++) strcpy(g_historico[i-1], g_historico[i]);
        strncpy(g_historico[g_hist_cap-1], s, BUF-1);
        g_historico[g_hist_cap-1][BUF-1] = '\0';
    }
}

static const char* obter_cmd_auto(void) {
    static int initialized = 0;
    static char bufcopy[1024];
    static char* cursor = NULL;
    if (!initialized) {
        const char* env = getenv("TORRE_CMDS");
        if (env && *env) {
            strncpy(bufcopy, env, sizeof(bufcopy)-1);
            bufcopy[sizeof(bufcopy)-1] = '\0';
            for (char* p = bufcopy; *p; ++p) if (*p == ';') *p = ',';
            cursor = bufcopy;
        }
        initialized = 1;
    }
    if (!cursor) return NULL;
    while (*cursor == ' ' || *cursor == '\t' || *cursor == ',') cursor++;
    if (*cursor == '\0') return NULL;
    char* start = cursor;
    while (*cursor && *cursor != ',' && *cursor != ';' && *cursor != ' ' && *cursor != '\t') cursor++;
    if (*cursor) { *cursor = '\0'; cursor++; }
    return start;
}

static SOCKET conectar_servidor(const char* host, int porta) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) die_wsa("socket");

    struct sockaddr_in addr;
    ZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((u_short)porta);
    unsigned long ip = inet_addr(host);
    if (ip == INADDR_NONE) die_wsa("inet_addr");
    addr.sin_addr.s_addr = ip;

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        die_wsa("connect");
    return sock;
}

static int receber_linha_cli(SOCKET s, char* buf, size_t cap) {
    size_t i = 0;
    while (i + 1 < cap) {
        char c;
        int r = recv(s, &c, 1, 0);
        if (r == 0) return 0;
        if (r == SOCKET_ERROR) return -1;
        if (c == '\n') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return (int)i;
}

static void parse_args_cliente(int argc, char** argv, int* hist_n, int* sem_cores) {
    *hist_n = 0;
    *sem_cores = 0;
    for (int i=2;i<argc;i++) {
        if (strcmp(argv[i], "--hist") == 0 && i+1<argc) { *hist_n = atoi(argv[++i]); }
        else if (strcmp(argv[i], "--sem-cores") == 0) { *sem_cores = 1; }
    }
}

int cliente_main(int argc, char** argv) {
    int hist_n = 0, sem_cores = 0;
    parse_args_cliente(argc, argv, &hist_n, &sem_cores);
    if (!sem_cores) habilitar_vt();
    SOCKET sock = conectar_servidor("127.0.0.1", PORTA);

    HANDLE hMap = NULL;
    for (int i=0; i<50 && !hMap; ++i) {
        hMap = OpenFileMappingW(FILE_MAP_READ, FALSE, MAP_NAME);
        if (!hMap) Sleep(100);
    }
    if (!hMap) die_last_error("OpenFileMappingW");
    EstadoTorre* st = (EstadoTorre*) MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, sizeof(EstadoTorre));
    if (!st) die_last_error("MapViewOfFile");

    int meu_id = 0;
    int ultimo_vez = 0, ultimo_evento = 0, p1 = 0, p2 = 0;
    int tempo_turno_ms = 0;
    g_hist_cap = (hist_n>0 && hist_n<=50)? hist_n : 0;
    g_hist_len = 0;
    char buf[BUF];

    printf("[CLIENTE] Conectado. Aguardando ID...\n");

    while (1) {
        int r = receber_linha_cli(sock, buf, sizeof(buf));
        if (r <= 0) {
            printf("[CLIENTE] Conexao encerrada.\n");
            break;
        }

        if (strncmp(buf, "ID ", 3) == 0) {
            meu_id = atoi(buf+3);
            printf("[CLIENTE] Voce e o Jogador %d.\n", meu_id);
        }
        else if (strncmp(buf, "MSG ", 4) == 0) {
            printf("%s\n", buf+4);
            hist_add(buf+4);
        }
        else if (strncmp(buf, "START", 5) == 0) {
            printf("[CLIENTE] Jogo iniciado. %s\n", buf);
            hist_add(buf);
        }
        else if (strncmp(buf, "VEZ ", 4) == 0) {
            int vez = atoi(buf+4);
            int evento = 0;
            int t = 0;
            const char* p = strstr(buf, "EVENTO ");
            if (p) evento = atoi(p + 7);
            const char* q = strstr(buf, "TIME ");
            if (q) t = atoi(q + 5);
            ultimo_vez = vez; ultimo_evento = evento; tempo_turno_ms = t;
            ui_desenhar_torre(st->altura, st->max_altura, meu_id, ultimo_vez, ultimo_evento, p1, p2, tempo_turno_ms);
            if (g_hist_cap) {
                printf("Ultimas acoes:\n");
                for (int i=0;i<g_hist_len;i++) printf("- %s\n", g_historico[i]);
                printf("\n");
            }
            if (vez == meu_id) {
                printf("Sua vez!\n");
                const char* auto_tok = obter_cmd_auto();
                if (auto_tok) {
                    char linha[BUF];
                    if ((auto_tok[0] >= '0' && auto_tok[0] <= '9') || (auto_tok[0] == '-' && auto_tok[1])) {
                        snprintf(linha, sizeof(linha), "ADD %s\n", auto_tok);
                    } else {
                        snprintf(linha, sizeof(linha), "%s\n", auto_tok);
                    }
                    printf("[AUTO] Enviando: %s", linha);
                    send(sock, linha, (int)strlen(linha), 0);
                } else {
                    printf("> ");
                    fflush(stdout);
                    char linha[BUF];
                    if (!fgets(linha, sizeof(linha), stdin)) strcpy(linha, "STATUS\n");
                    send(sock, linha, (int)strlen(linha), 0);
                }
            }
        }
        else if (strncmp(buf, "AGUARDE", 7) == 0) {
            printf("%s[CLIENTE]%s Aguardando sua vez...\n", C_LABEL(), C_RESET());
        }
        else if (strncmp(buf, "TORRE=", 6) == 0) {
            int altura = 0, vez = 0, p1n = p1, p2n = p2, evn = ultimo_evento;
            sscanf(buf, "TORRE=%d VEZ=%d P1=%d P2=%d EVT=%d", &altura, &vez, &p1n, &p2n, &evn);
            p1 = p1n; p2 = p2n; ultimo_evento = evn;
            ui_desenhar_torre(altura, st->max_altura, meu_id, vez, ultimo_evento, p1, p2, tempo_turno_ms);
            if (g_hist_cap) {
                printf("Ultimas acoes:\n");
                for (int i=0;i<g_hist_len;i++) printf("- %s\n", g_historico[i]);
                printf("\n");
            }
        }
        else if (strncmp(buf, "ERRO", 4) == 0) {
            printf("%s[CLIENTE]%s %s%s%s\n", C_LABEL(), C_RESET(), C_ERR(), buf, C_RESET());
            hist_add(buf);
        }
        else if (strncmp(buf, "FIM ", 4) == 0) {
            printf("[CLIENTE] %s\n", buf);
            int vencedor = 0, altura = 0, p1f = 0, p2f = 0;
            if (strncmp(buf, "FIM EMPATE", 10) == 0) {
                sscanf(buf, "FIM EMPATE ALTURA=%d P1=%d P2=%d", &altura, &p1f, &p2f);
                ui_desenhar_torre(altura, st->max_altura, meu_id, 0, 0, p1f, p2f, 0);
                printf("Resultado: EMPATE\n");
            } else {
                sscanf(buf, "FIM VENCEDOR=%d ALTURA=%d P1=%d P2=%d", &vencedor, &altura, &p1f, &p2f);
                ui_desenhar_torre(altura, st->max_altura, meu_id, 0, 0, p1f, p2f, 0);
                printf("Vencedor: Jogador %d\n", vencedor);
            }
            break;
        }
        else {
            printf("[CLIENTE] Servidor: %s\n", buf);
        }
    }

    UnmapViewOfFile(st);
    CloseHandle(hMap);
    closesocket(sock);
    return 0;
}
