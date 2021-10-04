#ifndef WINSYS_H
#define WINSYS_H

/*
cabecalho para inclusao de dados formatados
para compatbilidade universal entre dispositivos,

sendo este um nivel de compatbilidade para windows
*/

// bibliotecas
// para o exit()
#include <stdlib.h>
// para o strlen
#include <stdio.h>
// para testar a latencia
#include <time.h>

//funcoes para informacoes
#include "..//debug.h"
// variavel netdata
#include "netdata.h"

// para o uso de sockets
#include <winsock2.h>
#include <WS2tcpip.h>
// .dll necessaria
#pragma comment(lib, "ws2_32.lib")

// macros para definicao do funcionamento do sistema
constexpr bool SERVERSIDE = true;
constexpr bool CLIENTSIDE = false;

// macros para definir estado de conexao
constexpr bool SOCK_CONNECTED = true;
constexpr bool SOCK_UNCONNECTED = false;

// macros para diferenciacao de sockets e seus usos
constexpr int CONNECT_SOCKET = 0;
constexpr int LISTEN_SOCKET = 1;

struct Netlink {
private:
	// num sei oque e, mas precisa, entao ta ai kakakak
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
	int mypacklen = 0;
	int otherpacklen = 0;

	// estruturas para informacao de conexao
	struct addrinfo *sockaddr = nullptr, hints;

	int verifydata(int result) {

		if (result >= 0) return result;
		else {
			debugprint("nada foi enviado pelo outro lado da conexao, ocorrendo um erro");
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

	// referencia externa para wininit.h
	Netlink(bool, const char*, const char*, int);
	~Netlink();
	
	// recebe a data, em pacotes se for determinado valor != 0 para otherpacklen
	// recebe como argumento uma estrutura do tipo NETDATA, para manipulacao
	void recvdata(NETDATA *data) {

		// se o tamanho dos pacotes de dados nao forem determinados receba tudo de uma vez
		if (otherpacklen == 0) {
			// primeiro recebe o tamanho da data a ser alocada, em *bytes*
			verifydata(recv(sockets[CONNECT_SOCKET], (char*)&data->lenght, sizeof(data->lenght), NULL));

			//aloca um buffer para sustentar isso
			data->data = realloc(data->data, data->lenght);

			// e recebe os dados
			verifydata(recv(sockets[CONNECT_SOCKET], (char*)data->data, data->lenght, NULL));

		}
		else {
			// se ouver envio em pacotes de dados
			// cria uma estrutura para comportar os dados recebidos da conexao
			NETDATA packdata;
			
			// e insere-os na data principal, de acordo com o tipo de dados
			do {
				// recebe um pacote da data
				recvdata_packed(&packdata);

				//insere-o na data verdadeira
				data->lenght = DATA_PACK_MERGE(data, &packdata, otherpacklen);
			} while (data->loaded < data->packload);
		}

	}

	// funcao semelhante a recv, mas de uso esclusivo a dados enviados em pacotes
	// recebe como argumento uma estrutura do tipo NETDATA
	// assim podendo tratar separadamente os pacotes recebidos
	// o buffer é truncado a cada pacote recebido
	void recvdata_packed(NETDATA* data) {

		// recebe os pacotes ate o fim, se aver terminado ou for igual a 0 continua
		if (data->loaded < data->packload) {
			//recebe o pacote
			verifydata(recv(sockets[CONNECT_SOCKET], (char*)data->data, otherpacklen, NULL));
			// aumenta a quantidade de pacotes recebidos
			data->loaded++;

			return;
		}

		// se nao ouver envio em pacotes, nao faz sentido usar essa funcao
		if (otherpacklen == 0) {
			debugprint("nao a uso de pacotes de dados por parte da conexao");
			return;
		}

		// se nada foi recebido ainda
		if (data->loaded == data->packload == 0) {
			// recebe o numero de pacotes a serem recebidos
			verifydata(recv(sockets[CONNECT_SOCKET], (char*)&data->packload, sizeof(data->packload), NULL));

			//alloca o tamanho dos pacotes
			data->data = realloc(data->data, otherpacklen);

			// e refaza chamada a funcao para receber o 1 pacote
			recvdata_packed(data);
		}

		// se a operacao foi concluida limpe o buffer temporario
		if (data->loaded == data->packload != 0) {
			free(data->data);
			debugprint("recebimento de dados por pacotes finalizado");
		}
	}

	// envia dados a conexao
	void senddata(NETDATA *buff) {
		logprint("enviando dados");
		
		// verifica se nao foi inserido buffer nulo
		if (!buff || !buff->data) {
			debugprint("buffer nulo inserido como argumento\n");
			verifydata(-1);
		}

		// guarda o tamanho du buffer, se nao especificado faz isso sozinho
		if (buff->lenght == 0) {
			debugprint("tamanho do buffer nao informado\n");
			verifydata(-1);
		}
			

		//envia o tamanho do buffer a ser enviado
		verifydata(send(sockets[CONNECT_SOCKET], (char*)&buff->lenght, sizeof(buff->lenght), NULL));

		// e envia os dados
		buff->lenght = verifydata(send(sockets[CONNECT_SOCKET], (char*)buff->data, buff->lenght, NULL));
		

	}

	void senddata_packed() {

	}
	
};

#endif // WINSYS_H
