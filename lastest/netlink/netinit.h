#ifndef WININIT_H
#define WININIT_H

#include "netconnect.h"

// inicializa os recursos necessarios
Netlink::Netlink(bool linktype, int package = 0, const char* port = 0) {

	infoprint("iniciando rede como %s \n", ((linktype) ? "servidor" : "cliente"));

	// se o tamanho de pacote de dados for um valor nao operavel
	if (package < 0) debugerror(-4, "[erro] em <Netlink::Netlink()> argumento invalido");
	

	// define a forma de operacao de conexao
	this->linktype = linktype;

	//insere endereco da conexao
	//strcpy_s(listensocket.IP, ip); --------------------------------possivel causador de erro
	//strcpy_s(listensocket.port, port);

	// define o tamanho do pacote de dados
	sendpacklen = package;

	// inicia a winsock2.h na vesao 2.2
	compatibility_level_startup();

	// zera a memoria
	memset(&hints, 0, sizeof(hints));

	// molda o sistema baseado no tipo de operacao informada
	// cria o sistema como servidor
	if (linktype == SERVER_SIDE) {

		// cria o descritor de socket (linux, descritor de arquivo)
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// retorna um descritor de socket pronto para uso e linkado na porta *port* variavel
		verify(getaddrinfo(NULL, port, &hints, &sockaddr) != 0);

		// alloca o socket de escuta, esclusivo do servidor
		listensocket.socket = socket(sockaddr->ai_family, sockaddr->ai_socktype, sockaddr->ai_protocol);

		// verifica se ouve erro
		verify(listensocket.socket == ~0);


		infoprint("associando a porta: \"%s\" \n", port);
		// encaderna o socket na conexao
		verify(bind(listensocket.socket, sockaddr->ai_addr, sockaddr->ai_addrlen) != 0);
	
	
	}
	
	if (linktype == CLIENT_SIDE) {

		// cria o descritor de socket
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
	}
	if (sendpacklen) infoprint("conexao de envio de %d bytes por pacote\n", sendpacklen);
}

// destrutuor para liberar recursos
Netlink::~Netlink() {
	closeallsockets(sockethead);
	if (linktype == SERVER_SIDE) compatibility_level_closesocket(listensocket.socket);
	compatibility_level_cleanup();
	freeaddrinfo(sockaddr);
}

#endif // !WININIT_H