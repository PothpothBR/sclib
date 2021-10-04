#ifndef WINSYS_H
#define WINSYS_H

/*
Cabecalho para inclusao de funcoes para manipular
o envio de dados em redes para compatbilidade universal
entre dispositivos.

Sendo este um nivel de compatbilidade para windows.
*/

// fazer esse arquivo universal
// mover todos os metodos e variaveis da estrutura que sao compativeis com o windows para um arquivo separado 

// para o exit()
#include <stdlib.h>

// para o strlen
#include <stdio.h>

// para testar a latencia
#include <time.h>

//funcoes para informacoes
#include "..//debug.h"

// estrutura netdata
#include "..//netdata.h"

// para o uso de sockets
#include <winsock2.h>
#include <WS2tcpip.h>

// .dll necessaria para o uso de winsock2.h
#pragma comment(lib, "ws2_32.lib")

#include "wininit.h"
#include "winlink.h"

// macros para definicao do funcionamento do sistema
constexpr bool SERVERSIDE = true;
constexpr bool CLIENTSIDE = false;

// macros para definir estado de conexao
constexpr bool SOCK_CONNECTED = true;
constexpr bool SOCK_UNCONNECTED = false;

// macros para diferenciacao de sockets padrao e seus usos
// os sockets padrao sao conexoes pre-definidas ao iniciar o servidor
constexpr int CONNECT_SOCKET = 0;
constexpr int LISTEN_SOCKET = 1;

struct Netlink {
private:
	// num sei oque e, mas precisa, entao ta ai
	WSADATA wsadata;

	// armazena os sockets necessarios
	int *sockets = nullptr;

	// determina o tamanho do buffer de sockets
	int socketslen = 0;

	// define o estado de conexao
	bool connected = false;
	
	// informa se o sistema de conexao funcionara como servidor ou cliente
	bool linktype = true;

	// determina a quantidade maxima de dados enviados por vez, e util em casos de rede instavel,
	// pois nao perde todo o buffer transmitido, somente parte dele
	// taxa de dados por pacote ao enviar
	int sendpacklen = 0;
	// taxa de dados por pacote ao receber
	int recivepacklen = 0;

	// estruturas para informacao de conexao
	struct addrinfo *sockaddr = nullptr, hints;

	// verifica erros ao enviar ou receber
	int verifydata(int result) {

		if (result >= 0) return result;
		else {
			debugprint("erro ao enviar ou receber dados");
			verify(true);
			closesocket(sockets[CONNECT_SOCKET]);
			if (linktype == SERVERSIDE) closesocket(sockets[LISTEN_SOCKET]);
			free(sockets);
			close();
		}
	}

	// verifica erro critico e encerra o programa se necessario
	bool verify(bool cond) {
		if (cond) {
			infoprint("erro coletado: %d\n", WSAGetLastError());
			debugprint("encerrando .dll WSA pela funcao Netlink::verify()");
			WSACleanup();
		}
		return cond;
	}

	// encerramento simples do sistema
	void close() {
		debugprint("encerrando o programa antes do seu fim real");
		system("PAUSE");
		exit(0);
	}

public:

	// referencia externa para o contrutor e destrutor. wininit.h
	Netlink(bool, const char*, const char*, int);
	~Netlink();
	
	// recebe a data, em pacotes se for determinado valor != 0 para recivepacklen
	// recebe como argumento uma estrutura do tipo NETDATA, para manipulacao
	void recvdata(NETDATA*);

	// funcao semelhante a recv, mas de uso esclusivo a dados enviados em pacotes
	// recebe como argumento uma estrutura do tipo NETDATA
	// assim podendo tratar separadamente os pacotes recebidos
	// os dados sao inseridos em netdata->packdata
	// a funcao deve ser chamada a cada recebimento de dados
	// os dados recebidos a cada loop ficam armazenados em NETDATA.packdata[NETDATA.loaded - 1];
	void recvdata_packed(NETDATA*);

	// envia dados a conexao
	void senddata(NETDATA*);

	//envia os dados em pacotes
	void senddata_packed(NETDATA*);
};

#endif // WINSYS_H
