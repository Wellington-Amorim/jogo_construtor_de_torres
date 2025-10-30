#include "common.h"

static SOCKET criar_socket_servidor(int porta) {
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) die_wsa("socket");

	BOOL opt = TRUE;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) == SOCKET_ERROR)
		die_wsa("setsockopt");

	struct sockaddr_in addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons((u_short)porta);

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		die_wsa("bind");

	if (listen(sock, 2) == SOCKET_ERROR) die_wsa("listen");
	return sock;
}

static void enviar(SOCKET s, const char* msg) {
	size_t n = strlen(msg);
	int r = send(s, msg, (int)n, 0);
	if (r == SOCKET_ERROR) die_wsa("send");
}

static int receber_linha(SOCKET s, char* buf, size_t cap) {
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

static void parse_args_servidor(int argc, char** argv, int* tempo_ms, int* max_altura_cfg, int* permitir_risco, int* permitir_sabotagem, int* modo_treino, int* duracao_ms) {
	*tempo_ms = 30000;
	*max_altura_cfg = -1;
	*permitir_risco = 1;
	*permitir_sabotagem = 1;
	*modo_treino = 0;
	*duracao_ms = 0;
	for (int i=2;i<argc;i++) {
		if (strcmp(argv[i], "--tempo") == 0 && i+1<argc) { *tempo_ms = atoi(argv[++i]); }
		else if (strcmp(argv[i], "--max") == 0 && i+1<argc) { *max_altura_cfg = atoi(argv[++i]); }
		else if (strcmp(argv[i], "--sem-risco") == 0) { *permitir_risco = 0; }
		else if (strcmp(argv[i], "--sem-sabotagem") == 0) { *permitir_sabotagem = 0; }
		else if (strcmp(argv[i], "--modo") == 0 && i+1<argc) { if (strcmp(argv[i+1], "treino")==0) *modo_treino=1; i++; }
		else if (strcmp(argv[i], "--duracao") == 0 && i+1<argc) { *duracao_ms = atoi(argv[++i]); }
	}
}

int servidor_main(int argc, char** argv) {
	HANDLE hMutex = CreateMutexW(NULL, FALSE, MUTEX_NAME);
	if (!hMutex) die_last_error("CreateMutexW");
	HANDLE hMap = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(EstadoTorre), MAP_NAME);
	if (!hMap) die_last_error("CreateFileMappingW");
	EstadoTorre* st = (EstadoTorre*) MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(EstadoTorre));
	if (!st) die_last_error("MapViewOfFile");

	srand((unsigned int)GetTickCount());
	st->altura = 0;
	st->vez = 1;
	int tempo_cfg, max_cfg, permitir_risco, permitir_sabotagem, modo_treino, duracao_ms;
	parse_args_servidor(argc, argv, &tempo_cfg, &max_cfg, &permitir_risco, &permitir_sabotagem, &modo_treino, &duracao_ms);
	st->max_altura = (max_cfg>0? max_cfg : 30 + (rand() % 41));
	st->conectado[1] = st->conectado[2] = 0;
	st->pontos[1] = st->pontos[2] = 0;
	st->evento_atual = 0;
	st->tempo_turno_ms = tempo_cfg;
	st->vez = (rand()%2) ? 1 : 2;

	SOCKET srv = criar_socket_servidor(PORTA);
	printf("[SERVIDOR] Aguardando 2 jogadores em 127.0.0.1:%d...\n", PORTA);

	SOCKET clientes[3] = {0};
	struct sockaddr_in cliaddr;
	int clisz = (int)sizeof(cliaddr);

	for (int pid = 1; pid <= 2; ++pid) {
		SOCKET cfd = accept(srv, (struct sockaddr*)&cliaddr, &clisz);
		if (cfd == INVALID_SOCKET) die_wsa("accept");
		clientes[pid] = cfd;
		st->conectado[pid] = 1;
		char hello[BUF];
		snprintf(hello, sizeof(hello), "ID %d\n", pid);
		enviar(cfd, hello);
		enviar(cfd, "MSG Bem-vindo ao Construtor de Torres!\n");
		printf("[SERVIDOR] Jogador %d conectado.\n", pid);
	}

	for (int i=1;i<=2;i++) {
		char startmsg[BUF];
		snprintf(startmsg, sizeof(startmsg), "START MAX=%d\n", st->max_altura);
		enviar(clientes[i], startmsg);
	}

	int vencedor = 0;
	int empate = 0;
	DWORD inicio_ms = GetTickCount();

	while (!vencedor && !empate) {
		if (duracao_ms > 0) {
			DWORD agora = GetTickCount();
			if ((int)(agora - inicio_ms) >= duracao_ms) {
				if (st->pontos[1] > st->pontos[2]) vencedor = 1;
				else if (st->pontos[2] > st->pontos[1]) vencedor = 2;
				else empate = 1;
				break;
			}
		}

		int j = st->vez;
		SOCKET s_atual = clientes[j];
		SOCKET s_outro = clientes[j==1?2:1];

		int evt = rand() % 4;
		if (!permitir_risco && evt == 2) evt = 0;
		if (!permitir_sabotagem && evt == 3) evt = 0;
		if (modo_treino) {
			if (evt == 2 || evt == 3) evt = 0;
		}
		st->evento_atual = evt;
		char msg[BUF];
		snprintf(msg, sizeof(msg), "VEZ %d EVENTO %d TIME %d\n", j, st->evento_atual, st->tempo_turno_ms);
		enviar(s_atual, msg);
		enviar(s_outro, "AGUARDE\n");
		if (st->evento_atual == 1) enviar(s_atual, "MSG POWER-UP: DOUBLE! Sua proxima adicao vale em dobro.\n");
		else if (st->evento_atual == 2) enviar(s_atual, "MSG RISCO: Se voce adicionar 4 ou 5, ha 50% de chance da torre cair!\n");
		else if (st->evento_atual == 3) enviar(s_atual, "MSG SABOTAGEM: Voce pode usar 'SABOTAR Y' (1..5) para remover blocos.\n");

		char linha[BUF];
		DWORD timeout_ms = (DWORD)st->tempo_turno_ms;
		setsockopt(s_atual, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));
		int r = receber_linha(s_atual, linha, sizeof(linha));
		DWORD zero_timeout = 0;
		setsockopt(s_atual, SOL_SOCKET, SO_RCVTIMEO, (const char*)&zero_timeout, sizeof(zero_timeout));
		if (r <= 0) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("[SERVIDOR] Jogador %d estourou o tempo. Penalidade aplicada.\n", j);
				WaitForSingleObject(hMutex, INFINITE);
				if (st->altura > 0) st->altura -= 1;
				ReleaseMutex(hMutex);
				st->vez = (j==1?2:1);
				char msgp[BUF];
				snprintf(msgp, sizeof(msgp), "MSG Tempo esgotado para Jogador %d: -1 bloco.\n", j);
				enviar(clientes[1], msgp);
				enviar(clientes[2], msgp);
				char up[BUF];
				snprintf(up, sizeof(up), "TORRE=%d VEZ=%d P1=%d P2=%d EVT=%d\n", st->altura, st->vez, st->pontos[1], st->pontos[2], st->evento_atual);
				enviar(clientes[1], up);
				enviar(clientes[2], up);
				continue;
			} else {
				printf("[SERVIDOR] Jogador %d desconectou. Encerrando...\n", j);
				break;
			}
		}

		if (strncmp(linha, "ADD ", 4) == 0) {
			int x = atoi(linha+4);
			if (x <= 0) x = 1;
			if (x > 5) x = 5;
			if (st->evento_atual == 1) {
				x *= 2;
			} else if (st->evento_atual == 2) {
				if (x >= 4) {
					if ((rand() % 100) < 50) {
						enviar(s_atual, "ERRO Torre desabou! Falha de sincronizacao.\n");
						enviar(clientes[j==1?2:1], "ERRO Torre desabou! Falha de sincronizacao.\n");
						char fim[BUF];
						snprintf(fim, sizeof(fim), "FIM VENCEDOR=%d ALTURA=%d\n", (j==1?2:1), st->altura);
						enviar(clientes[1], fim);
						enviar(clientes[2], fim);
						printf("[SERVIDOR] Torre desabou por risco. Vencedor: Jogador %d.\n", (j==1?2:1));
						vencedor = (j==1?2:1);
						break;
					}
				}
			}
			WaitForSingleObject(hMutex, INFINITE);
			st->altura += x;
			st->pontos[j] += x;
			if (st->altura >= st->max_altura) {
				st->altura = st->max_altura;
				vencedor = j;
			}
			ReleaseMutex(hMutex);

			st->vez = (j==1?2:1);

			char up[BUF];
			snprintf(up, sizeof(up), "TORRE=%d VEZ=%d P1=%d P2=%d EVT=%d\n", st->altura, st->vez, st->pontos[1], st->pontos[2], st->evento_atual);
			enviar(clientes[1], up);
			enviar(clientes[2], up);
		}
		else if (strncmp(linha, "SABOTAR ", 8) == 0) {
			int y = atoi(linha+8);
			if (y <= 0) y = 1;
			if (y > 5) y = 5;
			if (st->evento_atual != 3) {
				enviar(s_atual, "ERRO Nao ha evento de sabotagem nesta rodada.\n");
			} else {
				WaitForSingleObject(hMutex, INFINITE);
				st->altura -= y;
				if (st->altura < 0) st->altura = 0;
				ReleaseMutex(hMutex);
				st->vez = (j==1?2:1);
				char up[BUF];
				snprintf(up, sizeof(up), "TORRE=%d VEZ=%d P1=%d P2=%d EVT=%d\n", st->altura, st->vez, st->pontos[1], st->pontos[2], st->evento_atual);
				enviar(clientes[1], up);
				enviar(clientes[2], up);
			}
		}
		else if (strncmp(linha, "STATUS", 6) == 0) {
			char stmsg[BUF];
			snprintf(stmsg, sizeof(stmsg), "TORRE=%d VEZ=%d MAX=%d P1=%d P2=%d EVT=%d\n", st->altura, st->vez, st->max_altura, st->pontos[1], st->pontos[2], st->evento_atual);
			enviar(s_atual, stmsg);
		}
		else if (strncmp(linha, "SAIR", 4) == 0) {
			printf("[SERVIDOR] Jogador %d saiu.\n", j);
			break;
		} else {
			enviar(s_atual, "ERRO Comando invalido. Use: ADD X | SABOTAR Y | STATUS | SAIR\n");
		}
	}

	if (empate) {
		char fim[BUF];
		snprintf(fim, sizeof(fim), "FIM EMPATE ALTURA=%d P1=%d P2=%d\n", st->altura, st->pontos[1], st->pontos[2]);
		enviar(clientes[1], fim);
		enviar(clientes[2], fim);
		printf("[SERVIDOR] Jogo encerrado. EMPATE. Placar: P1=%d P2=%d\n", st->pontos[1], st->pontos[2]);
	}
	else if (vencedor) {
		char fim[BUF];
		snprintf(fim, sizeof(fim), "FIM VENCEDOR=%d ALTURA=%d P1=%d P2=%d\n", vencedor, st->altura, st->pontos[1], st->pontos[2]);
		enviar(clientes[1], fim);
		enviar(clientes[2], fim);
		printf("[SERVIDOR] Jogo encerrado. Vencedor: Jogador %d. Placar: P1=%d P2=%d\n", vencedor, st->pontos[1], st->pontos[2]);
	}

	for (int i=1;i<=2;i++) if (clientes[i] != 0) closesocket(clientes[i]);
	closesocket(srv);

	UnmapViewOfFile(st);
	CloseHandle(hMap);
	CloseHandle(hMutex);
	return 0;
}
