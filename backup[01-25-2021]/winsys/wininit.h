#ifndef WININIT_H
#define WININIT_H
#include "winlink.h"
#include <iostream>
using namespace std;
// inicializa os recursos necessarios
Netlink::Netlink(bool linktype, const char* port, const char* ip = "localhost", int package = 0) {

	infoprint("iniciando rede como %s \n", ((linktype) ? "servidor" : "cliente"));

	// se o tamanho de pacote de dados for um valor nao operavel
	if (package < 0) {
		infoprint("tamanho de pacote de dados incorreto");
		exit(0);
	}

	// define a forma de operacao de conexao
	// true == SERVERSIDE
	// false == CLIENTSIDE
	this->linktype = linktype;

	// define o tamanho do pacote de dados
	// se o tamanho do pacote de dados for 0, o envio sera de forma direta
	// enviando o tamanho total de uma vez so
	sendpacklen = package;

	// inicia a winsock2.h na vesao 2.2
	if (verify(WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)) close();

	// inicializa a memoria de hints em zero
	ZeroMemory(&hints, sizeof(hints));

	// molda o sistema baseado no tipo de operacao informada
	if (linktype == SERVERSIDE) {
		// cria o sistema como servidor

		// se o ip for definido, insere um alerta de nao uso do dado
		if (ip != "localhost") {
			debugprint("\nignored ip, because it is a server connection\n");
		}

		//define o numero de sockets como 2, 1 para o listen e outro para o connect
		socketslen = 2;

		// cria o descritor de socket (linux, descritor de arquivo)
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// retorna um descritor de socket proto para uso e linkado na porta *port* variavel
		if (verify(getaddrinfo(NULL, port, &hints, &sockaddr) != 0)) exit(0);
	}
	// cria o sistema como cliente
	else if (linktype == CLIENTSIDE) {


		//define o numero de sockets como 1, para o connect
		socketslen = 1;

		if (ip == "") {
			infoprint("\nserver ip not informed\n");
			WSACleanup();
			close();
		}

		// cria o descritor de socket
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// retorna um descritor de socket
		if (verify(getaddrinfo(ip, port, &hints, &sockaddr) != 0)) exit(0);
	}

	// realloca aumentando o tamanho de acordo com o tipo do sistema de conexao
	int* sockvalid = (int*)calloc(sizeof(int), socketslen);

	// verifica possivel retorno nulo
	if (!sockvalid) {
		infoprint("\nnull memory return by realloc\n");
		WSACleanup();
		for (; socketslen > 0; socketslen--) closesocket(sockets[socketslen - 1]);
		free(sockets);
		freeaddrinfo(sockaddr);
		close();
	}

	// se tudo ocorrer bem insira o retono a variavel e retorne o tamanho final, o qual corresponde a posicao do socket
	sockets = sockvalid;

	// inicialize a zero
	for (int i = 0; i < socketslen; i++)sockets[i] = INVALID_SOCKET;

	// pelo lado do servidor encaderna o socket e aguarda conexao, se sucesso, conecta
	if (linktype == SERVERSIDE) {

		logprint("\n* dados brutos de inicializacao:\n*linktype %d\n*port %s\n*ip %s\n*WSA version 2.2\n*family %s\n*socktype %s\n*protocol %s\n*flags %s\n*ip not used\n\n",
			linktype, port, ip, "AF_INET", "SOCK_STREAM", "IPPROTO_TCP", "AI_PASSIVE");

		// alloca o primeiro socket, no caso do servidor, esse sera o LISTEN_SOCKET
		sockets[LISTEN_SOCKET] = socket(sockaddr->ai_family, sockaddr->ai_socktype, sockaddr->ai_protocol);

		// verifica se ouve erro
		if (verify(sockets[LISTEN_SOCKET] == INVALID_SOCKET)) {
			freeaddrinfo(sockaddr);
			close();
		}

		infoprint("associando a porta: \"%s\" \n", port);
		// encaderna o socket na conexao
		if (verify(bind(sockets[LISTEN_SOCKET], sockaddr->ai_addr, sockaddr->ai_addrlen) != 0)) {
			freeaddrinfo(sockaddr);
			closesocket(sockets[LISTEN_SOCKET]);
			free(sockets);
			close();
		}

		// libera as informacoes de endereco
		freeaddrinfo(sockaddr);

		infoprint("aguardando pedido de conexao \n");
		//aguarda pedido de conexao
		if (verify(listen(sockets[LISTEN_SOCKET], SOMAXCONN) == SOCKET_ERROR)) {
			closesocket(sockets[LISTEN_SOCKET]);
			free(sockets);
			close();
		}

		// aceita a conexao
		sockets[CONNECT_SOCKET] = accept(sockets[LISTEN_SOCKET], NULL, NULL);

		//verifica o estado da conexao
		if (verify(sockets[CONNECT_SOCKET] == SOCKET_ERROR)) {
			closesocket(sockets[CONNECT_SOCKET]);
			closesocket(sockets[LISTEN_SOCKET]);
			free(sockets);
			close();
		}

		infoprint("conectado\n");

		logprint("o tempo medido nao e um valor totalmente confiavel\n");


		infoprint("testando comunicacao \n");

		// mede o tempo de resposta
		clock_t Ticks[2];
		Ticks[0] = clock();

		// envia o tamanho do envio de pacote de dados maximo
		verifydata(send(sockets[CONNECT_SOCKET], (char*)&sendpacklen, sizeof(sendpacklen), NULL));

		// recebe o mesmo, so que o do outro lado conectado
		verifydata(recv(sockets[CONNECT_SOCKET], (char*)&recivepacklen, sizeof(recivepacklen), NULL));

		//O código a ter seu tempo de execução medido ficaria neste ponto.
		Ticks[1] = clock();
		int Tempo = (Ticks[1] - Ticks[0]) / CLOCKS_PER_SEC;

		infoprint("tempo de resposta %d ms\n", Tempo);

		// sinaliza o socket como conectado
		connected = true;
	}
	// pelo lado do cliente tenta conexao com o servidor
	else if (linktype == CLIENTSIDE) {

		logprint("\n* dados brutos de inicializacao:\n*linktype %d\n*port %s\n*ip %s\n*WSA version 2.2\n*family %s\n*socktype %s\n*protocol %s\n*flags NULL\n\n",
			linktype, port, ip, "AF_UNSPEC", "SOCK_STREAM", "IPPROTO_TCP");

		struct addrinfo* ptr = nullptr;
		bool connresult = false;

		// tenta a conexao 5 vezes
		for (int i = 4; i >= 0; i--) {


			infoprint("conectando a \"%s:%s\" \n", ip, port);
			// tenta conexao com IPv4 e IPv6
			char sipv[2][5] = { "IPv6", "IPv4" };
			int sipvi = 0;
			for (ptr = sockaddr; ptr != NULL; ptr = ptr->ai_next) {
				debugprint("tentando conexao como %s\n", sipv[sipvi]);
				// alloca o primeiro socket, no caso do cliente, esse sera o CONNECT_SOCKET
				sockets[CONNECT_SOCKET] = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

				if (connect(sockets[CONNECT_SOCKET], ptr->ai_addr, ptr->ai_addrlen) == INVALID_SOCKET) {



					closesocket(sockets[CONNECT_SOCKET]);
					sockets[CONNECT_SOCKET] = INVALID_SOCKET;

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
			// se sucesso pare o loop, snao aguarde 1 segundo e tente novamente
			if (connresult) {
				infoprint("falha ao conectar, tentando mais %d vezes\n", i);
				Sleep(1000);
			}
			else {
				infoprint("conectado\n");
				break;
			}

			// se falhar 4 vezer encerre o programa
			if (i <= 0) {
				WSACleanup();
				freeaddrinfo(sockaddr);
				closesocket(sockets[CONNECT_SOCKET]);
				free(sockets);
				infoprint("impossivel de conectar encerrando\n");
				close();
			}
		}
		// libera as informacoes de endereco
		freeaddrinfo(sockaddr);

		logprint("o tempo medido nao e um valor totalmente confiavel\n");

		infoprint("testando comunicacao \n");


		// mede o tempo de resposta
		clock_t Ticks[2];
		Ticks[0] = clock();

		// recebe o mesmo, so que o do outro lado conectado
		recv(sockets[CONNECT_SOCKET], (char*)&recivepacklen, sizeof(recivepacklen), NULL);

		// envia o tamanho do envio de pacote de dados maximo
		send(sockets[CONNECT_SOCKET], (char*)&sendpacklen, sizeof(sendpacklen), NULL);

		//O código a ter seu tempo de execução medido ficaria neste ponto.
		Ticks[1] = clock();
		double Tempo = (Ticks[1] - Ticks[0]) / CLOCKS_PER_SEC;

		infoprint("tempo de resposta %d ms\n", Tempo);

		// sinaliza o socket como conectado
		connected = true;
	}

	if (recivepacklen) infoprint("receber %d bytes por pacote\n", recivepacklen);
	if (sendpacklen) infoprint("enviar %d bytes por pacote\n", sendpacklen);

}

// destrutuor para liberar recursos
Netlink::~Netlink() {
	WSACleanup();
	closesocket(sockets[CONNECT_SOCKET]);
	if (linktype == SERVERSIDE) closesocket(sockets[LISTEN_SOCKET]);
	free(sockets);
}

#endif // !WININIT_H
