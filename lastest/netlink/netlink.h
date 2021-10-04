#ifndef WINSYS_H
#define WINSYS_H

// estrutura netdata
#include "netsocket.h"

// macros para definicao do funcionamento do sistema
constexpr bool SERVER_SIDE = true;
constexpr bool CLIENT_SIDE = false;

struct Netlink {
private:
	// num sei oque e, mas precisa, entao ta ai
	compatibility_level_data;

	// armazena os sockets e informacoes do mesmo
	NETSOCKET *sockethead = nullptr;
	NETSOCKET *socketfoot = nullptr;

	// armazena o socket para modo escuta em sistema de servidor, esse socket é morto ao iniciar em modo servidor
	NETSOCKET listensocket;

	// determina o tamanho do buffer de sockets
	int socketslen = 0;

	// informa se o sistema de conexao funcionara como servidor ou cliente
	bool linktype = true;

	// determina a quantidade maxima de dados enviados por vez, e util em casos de rede instavel,
	// pois nao perde todo o buffer transmitido, somente parte dele
	// taxa de dados por pacote ao enviar
	int sendpacklen = 0;

	// estruturas para informacao de conexao
	struct addrinfo *sockaddr = nullptr, hints;

	// verifica erro da biblioteca 
	static bool verify(int result) {
		if (result < 0) debugerror(-6, "[erro] da biblioteca do sistema: %d\n", compatibility_level_geterror());
		return result;
	}

public:

	//construtor padrao
	Netlink() {
	}

	// referencia externa para o contrutor e destrutor
	Netlink(bool, int, const char*);
	~Netlink();
	
	// recebe a data, em pacotes se for determinado valor != 0 para recivepacklen
	// recebe como argumento uma estrutura do tipo NETDATA, para manipulacao
	int recvdata(NETDATA*, int);

	// funcao semelhante a recv, mas de uso esclusivo a dados enviados em pacotes
	// recebe como argumento uma estrutura do tipo NETDATA
	// assim podendo tratar separadamente os pacotes recebidos
	// os dados sao inseridos em netdata->packdata
	// a funcao deve ser chamada a cada recebimento de dados
	// os dados recebidos a cada loop ficam armazenados em NETDATA.packdata[NETDATA.loaded - 1];
	int recvdata_packed(NETDATA*, int);

	// envia dados a conexao
	int senddata(NETDATA*, int);

	//envia os dados em pacotes
	int senddata_packed(NETDATA*, int);

	// conecta o sistema a alguem
	// a conexao fica em uma lista encadeada 
	int waitconnection(const char*, const char*);

	// retorna o ultimo socket conectado
	NETSOCKET* lastsocket() { return sockethead; }

	// retorna o id do ultimo socket, sendo possivel modificalo
	int& lastsocketid() { return sockethead->ID; }

	// retorna o ip do ultimo socket, somente leitura
	const char* const lastsocketip() { return sockethead->IP; }

	// muda o ID do socket especifico
	int changesockid(int ID, int newID) { getsocket(ID, sockethead)->ID = newID; }

	// retorna o socket pelo seu ID
	NETSOCKET* getsocketbyid(int ID) { return getsocket(ID, sockethead); }
};



#endif // WINSYS_H
