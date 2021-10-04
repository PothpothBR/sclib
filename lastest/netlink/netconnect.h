#ifndef NETCONNECT_H
#define NETCONNECT_H

#include "netio.h"

int Netlink::waitconnection(const char *port = 0, const char *ip = "localhost") {

	// se o sistema de conexao funciona como servidor
	if (linktype == SERVER_SIDE) {

		infoprint("aguardando pedido de conexao \n");

		//aguarda pedido de conexao
		int r = verify(listen(listensocket.socket, 2147483647)); if (r < 0) return r;

		//cria uma estrutura para as informacoes do socket
		struct sockaddr_in connectaddr;
		socklen_t connectaddrlen = sizeof(struct sockaddr_in);

		//cria uma estrutura para alocar o socket
		r = addsocket(socketslen + 1000, &sockethead, &socketfoot, &socketslen); if (r < 0) return r;

		// aceita a conexao
		sockethead->socket = (int) accept(listensocket.socket, (struct sockaddr*) &connectaddr, &connectaddrlen);

		//------------------------------------------------------------------

		//verifica o estado da conexao
		r = verify(sockethead->socket); if (r < 0) return r;

		strcpy(sockethead->IP, inet_ntoa(connectaddr.sin_addr));

		char buffer[10];
	
		sprintf(buffer, "%i", ntohs(connectaddr.sin_port));

		strcpy(sockethead->port, buffer);
		

		infoprint("conectado\n");

		infoprint("testando conexao \n");
		logprint("o tempo medido nao e um valor totalmente confiavel\n");

		// mede o tempo de resposta
		clock_t Ticks[2];
		Ticks[0] = clock();

		// envia o tamanho do envio de pacote de dados maximo
		r = verify(send(sockethead->socket, (char*)&sendpacklen, sizeof(sendpacklen), 0)); if (r < 0) return r;

		// recebe o mesmo, so que o do outro lado conectado
		r = verify(recv(sockethead->socket, (char*)&sockethead->datapacklen, sizeof(sockethead->datapacklen), 0)); if (r < 0) return r;

		//O código a ter seu tempo de execução medido ficaria neste ponto.
		Ticks[1] = clock();
		int Tempo = (Ticks[1] - Ticks[0]) / CLOCKS_PER_SEC;

		infoprint("tempo de resposta %d ms\n", Tempo);

		//inserea latencia de conexao
		sockethead->ms = Tempo;

		// sinaliza o socket como conectado
		sockethead->connected = true;

		if (sockethead->datapacklen) infoprint("conexao de recebimento de %d bytes por pacote\n", sockethead->datapacklen);

	}

	// pelo lado do cliente tenta conexao com o servidor
	else if (linktype == CLIENT_SIDE) {

		if (ip == "") debugerror(-4, "[erro] em <Netlink::waitconnection()> <const char* ip> argumento invalido");

		// informa que a conexao sera estabelecida localmente
		// necessario, pois o valor padrao é local host
		if (ip == "localhost") logprint("server ip defined witch \"localhost\"\n");

		// retorna um descritor de socket
		int r = verify(getaddrinfo(ip, port, &hints, &sockaddr) != 0); if (r < 0) return r;

		//armazena o resultado da tentativa de conexao
		bool connresult = false;

		// estrutura para armazenar dados do outro lado da conexao
		struct addrinfo connectaddr;
		//aguardar finalizar sistema para descobrir comousar isso-----------------------------------------

		// aloca um novo socket
		r = addsocket(socketslen + 1000, &sockethead, &socketfoot, &socketslen); if (r < 0) return r;

		// tenta a conexao 3 vezes
		for (int i = 2; i >= 0; i--) {

			infoprint("conectando a \"%s:%s\" \n", ip, port);

			// vaiaveis para exibir informacoes de conexao
			char sipv[2][5] = { "IPv6", "IPv4" };
			int sipvi = 0;

			// tenta conexao com IPv4 e IPv6
			for (struct addrinfo* ptr = sockaddr; ptr != NULL; ptr = ptr->ai_next) {
				logprint("tentando conexao como %s\n", sipv[sipvi]);
				// cria o primeiro socket, no caso do cliente, esse sera o CONNECT_SOCKET
				sockethead->socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				if ((int) connect(sockethead->socket, ptr->ai_addr, ptr->ai_addrlen) == ~0) {

					compatibility_level_closesocket(sockethead->socket);
					sockethead->socket = (int)INVALID_SOCKET;

					connresult = true;
					logprint("falhou\n");
				}
				else {

					connresult = false;
					logprint("sucesso\n");
					break;
				}
				sipvi++;
			}
			// se sucesso pare o loop, snao aguarde 0.5 segundo e tente novamente
			if (connresult) {
				logprint("falha ao conectar, tentando mais %d vezes\n", i);
				Sleep(500);
			}
			else {
				infoprint("conectado\n");
				break;
			}

			// se falhar 4 vezer encerre o programa
			if (i <= 0) debugerror(-4, "[erro] em <int Netlink::waitconnection()> <const char* port, const char* ip> argumento invalido");
		}

		logprint("o tempo medido nao e um valor totalmente confiavel\n");

		infoprint("testando conexao \n");

		// mede o tempo de resposta
		clock_t Ticks[2];
		Ticks[0] = clock();

		// recebe o mesmo, so que o do outro lado conectado
		r = verify(recv(sockethead->socket, (char*)&sockethead->datapacklen, sizeof(sockethead->datapacklen), 0)); if (r < 0) return r;

		// envia o tamanho do envio de pacote de dados maximo
		r = verify(send(sockethead->socket, (char*)&sendpacklen, sizeof(sendpacklen), 0)); if (r < 0) return r;

		//O código a ter seu tempo de execução medido ficaria neste ponto.
		Ticks[1] = clock();
		double Tempo = (Ticks[1] - Ticks[0]) / CLOCKS_PER_SEC;

		infoprint("tempo de resposta %d ms\n", Tempo);

		// sinaliza o socket como conectado
		sockethead->connected = true;
		if (sockethead->datapacklen) logprint("conexao de recebimento de %d bytes por pacote\n", sockethead->datapacklen);

		strcpy(sockethead->IP, ip);
		strcpy(sockethead->port, port);

	}


}

#endif // NETCONNECT_H