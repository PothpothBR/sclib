#ifndef NETCONNECT_H
#define NETCONNECT_H

#include "netio.h"

void Netlink::waitconnection(const char *port = 0, const char *ip = "localhost") {



	// se o sistema de conexao funciona como servidor
	if (linktype == SERVER_SIDE) {

		infoprint("aguardando pedido de conexao \n");

		//aguarda pedido de conexao
		if (verify(listen(listensocket.socket, SOMAXCONN) == SOCKET_ERROR)) {
			closeallsockets(this);
			shutdown();
		}

		struct addrinfo connectaddr;
		memset(&connectaddr, 0, sizeof(connectaddr));
		//aguardar finalizar sistema para descobrir comousar isso-----------------------------------------

		addsocket(this, socketslen + 1000);

		NETSOCKET* connectsocket = getsocket(this, socketslen + 1000 - 1);

		// aceita a conexao
		connectsocket->socket = accept(listensocket.socket, connectaddr.ai_addr, (int*)connectaddr.ai_addrlen);

		//verifica o estado da conexao
		if (connectsocket->socket == SOCKET_ERROR) {
			closeallsockets(this);
			shutdown();
		}

		// copia o endereco de ip
		//strcpy_s(connectsocket->IP, connectaddr.ai_addr->sa_data); -----------------------possivel causador de erro

		infoprint("conectado\n");

		infoprint("testando conexao \n");
		logprint("o tempo medido nao e um valor totalmente confiavel\n");

		// mede o tempo de resposta
		clock_t Ticks[2];
		Ticks[0] = clock();

		// envia o tamanho do envio de pacote de dados maximo
		verifydata(this, send(connectsocket->socket, (char*)&sendpacklen, sizeof(sendpacklen), 0));

		// recebe o mesmo, so que o do outro lado conectado
		verifydata(this, recv(connectsocket->socket, (char*)&connectsocket->datapacklen, sizeof(connectsocket->datapacklen), 0));

		//O código a ter seu tempo de execução medido ficaria neste ponto.
		Ticks[1] = clock();
		int Tempo = (Ticks[1] - Ticks[0]) / CLOCKS_PER_SEC;

		infoprint("tempo de resposta %d ms\n", Tempo);

		//inserea latencia de conexao
		connectsocket->ms = Tempo;

		// sinaliza o socket como conectado
		connectsocket->connected = true;

		if (connectsocket->datapacklen) infoprint("conexao de recebimento de %d bytes por pacote\n", connectsocket->datapacklen);

	}

	// pelo lado do cliente tenta conexao com o servidor
	else if (linktype == CLIENT_SIDE) {

		if (ip == "") {
			infoprint("server ip not informed\n");
			compatbility_level_cleanup();
			shutdown();
		}
		// informa que a conexao sera estabelecida localmente
		// necessario, pois o valor padrao é local host
		if (ip == "localhost") debugprint("server ip defined witch \"localhost\"\n");

		// retorna um descritor de socket
		if (verify(getaddrinfo(ip, port, &hints, &sockaddr) != 0)) exit(0);

		//armazena o resultado da tentativa de conexao
		bool connresult = false;

		// estrutura para armazenar dados do outro lado da conexao
		struct addrinfo connectaddr;
		memset(&connectaddr, 0, sizeof(connectaddr));
		//aguardar finalizar sistema para descobrir comousar isso-----------------------------------------

		// aloca um novo socket
		addsocket(this, socketslen + 1000);

		NETSOCKET* connectsocket = getsocket(this, socketslen + 1000 - 1);

		// tenta a conexao 3 vezes
		for (int i = 2; i >= 0; i--) {

			infoprint("conectando a \"%s:%s\" \n", ip, port);

			// vaiaveis para exibir informacoes de conexao
			char sipv[2][5] = { "IPv6", "IPv4" };
			int sipvi = 0;

			// tenta conexao com IPv4 e IPv6
			for (struct addrinfo* ptr = sockaddr; ptr != NULL; ptr = ptr->ai_next) {
				debugprint("tentando conexao como %s\n", sipv[sipvi]);
				// cria o primeiro socket, no caso do cliente, esse sera o CONNECT_SOCKET
				connectsocket->socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

				if (connect(connectsocket->socket, ptr->ai_addr, ptr->ai_addrlen) == INVALID_SOCKET) {

					compatbility_level_closesocket(connectsocket->socket);
					connectsocket->socket = (int)INVALID_SOCKET;

					connresult = true;
					debugprint("falhou\n");
				}
				else {

					connresult = false;
					debugprint("sucesso\n");
					break;
				}
				sipvi++;
			}
			// se sucesso pare o loop, snao aguarde 0.5 segundo e tente novamente
			if (connresult) {
				infoprint("falha ao conectar, tentando mais %d vezes\n", i);
				Sleep(500);
			}
			else {
				infoprint("conectado\n");
				break;
			}

			// se falhar 4 vezer encerre o programa
			if (i <= 0) {
				compatbility_level_cleanup();
				freeaddrinfo(sockaddr);
				closeallsockets(this);
				infoprint("impossivel de conectar encerrando\n");
				shutdown();
			}
		}

		logprint("o tempo medido nao e um valor totalmente confiavel\n");

		infoprint("testando conexao \n");

		// mede o tempo de resposta
		clock_t Ticks[2];
		Ticks[0] = clock();

		// recebe o mesmo, so que o do outro lado conectado
		recv(connectsocket->socket, (char*)&connectsocket->datapacklen, sizeof(connectsocket->datapacklen), 0);

		// envia o tamanho do envio de pacote de dados maximo
		send(connectsocket->socket, (char*)&sendpacklen, sizeof(sendpacklen), 0);

		//O código a ter seu tempo de execução medido ficaria neste ponto.
		Ticks[1] = clock();
		double Tempo = (Ticks[1] - Ticks[0]) / CLOCKS_PER_SEC;

		infoprint("tempo de resposta %d ms\n", Tempo);

		// sinaliza o socket como conectado
		connectsocket->connected = true;
		if (connectsocket->datapacklen) infoprint("conexao de recebimento de %d bytes por pacote\n", connectsocket->datapacklen);

	}


}

#endif // NETCONNECT_H