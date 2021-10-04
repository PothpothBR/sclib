#ifndef WINSYS_H
#define WINSYS_H

/*
Cabecalho para inclusao de funcoes para manipular
o envio de dados em redes para compatbilidade universal
entre dispositivos.

Sendo este um nivel de compatbilidade para windows.
*/

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
	void recvdata(NETDATA *data) {

		// verifica se nao foi inserido buffer nulo
		if (data == nullptr || data->data == nullptr) {
			infoprint("<Netlink::senddata()> buffer nulo inserido como argumento\n");
			verifydata(-1);
		}

		// verifica se a um valor de tamanho dos dados
		if (data->lenght == 0) {
			infoprint("<Netlink::senddata()> tamanho do buffer nao informado\n");
			verifydata(-1);
		}

		// se o tamanho dos pacotes de dados nao forem determinados receba tudo de uma vez
		if (recivepacklen == 0) {

			// primeiro recebe o tamanho da data a ser alocada, em *bytes*
			verifydata(recv(sockets[CONNECT_SOCKET], (char*)&data->lenght, sizeof(data->lenght), NULL));

			//aloca um buffer para sustentar isso
			data->data = realloc(data->data, data->lenght);

			// e recebe os dados
			verifydata(recv(sockets[CONNECT_SOCKET], (char*)data->data, data->lenght, NULL));

		}
		// se ouver envio em pacotes de dados
		else if(recivepacklen > 0){
			
			// recebe os dados
			do recvdata_packed(data); while (data->loaded < data->packload);

			// e os insere na data
			DATA_PACK_MERGE_ALL(data, recivepacklen);
		}
		// se ouver um erro na variavel Netlink.sendpacklen encerre
		else {
			infoprint("<Netlink::senddata()> tamanho do pacote de dados negativo Netlink.sendpacklen\n");
			verifydata(-1);
		}

	}

	// funcao semelhante a recv, mas de uso esclusivo a dados enviados em pacotes
	// recebe como argumento uma estrutura do tipo NETDATA
	// assim podendo tratar separadamente os pacotes recebidos
	// os dados sao inseridos em netdata->packdata
	// a funcao deve ser chamada a cada recebimento de dados
	// os dados recebidos a cada loop ficam armazenados em NETDATA.packdata[NETDATA.loaded - 1];
	void recvdata_packed(NETDATA* data) {

		// recebe os pacotes ate o fim, se haver terminado, ou for igual a 0. continua.
		if (data->loaded < data->packload) {

			//recebe o pacote
			if (data->overflowpacklen == 0) verifydata(recv(sockets[CONNECT_SOCKET], (char*)data->packdata[data->loaded], recivepacklen, NULL));

			// se ouver data quebrada
			else if (data->overflowpacklen >= 0) {

				// e nao for o ultimo pacote executa normalmente
				if (data->loaded < data->packload) verifydata(recv(sockets[CONNECT_SOCKET], (char*)data->packdata[data->loaded], recivepacklen, NULL));

				// se for o ultimo, recebe somente o tamanho do pacote restante
				else if (data->loaded == data->packload) verifydata(recv(sockets[CONNECT_SOCKET], (char*)data->packdata[data->loaded], data->overflowpacklen, NULL));
			}

			// aumenta a quantidade de pacotes recebidos
			data->loaded++;

			// encerra a funcao
			return;
		}

		// se nao ouver envio em pacotes, nao faz sentido usar essa funcao, encerra-a e retorna um aviso
		else if (recivepacklen == 0) {
			debugprint("nao a uso de pacotes de dados por parte da conexao");
			return;
		}

		// se nada foi recebido ainda
		else if (data->loaded == data->packload != 0) {
			
			// recebe o tamanho da data a ser recebida
			verifydata(recv(sockets[CONNECT_SOCKET], (char*)&data->lenght, sizeof(data->lenght), NULL));

			//alloca os pacotes, para serem recebidos e pre inicia os dados para alocacao
			DATA_PACK_CLEAN(data, recivepacklen);

			// e refaz a chamada a funcao para receber o 1 pacote
			recvdata_packed(data);
		}

		// se a operacao foi concluida limpe o buffer temporario
		else if (data->loaded == data->packload == 0) {
			free(data->data);
			debugprint("recebimento de dados por pacotes finalizado");
		}
	}

	// envia dados a conexao
	void senddata(NETDATA * data) {
		logprint("enviando dados");
		
		// verifica se nao foi inserido buffer nulo
		if (data == nullptr || data->data == nullptr) {
			infoprint("<Netlink::senddata()> buffer nulo inserido como argumento\n");
			verifydata(-1);
		}

		// verifica se a um valor de tamanho dos dados
		if (data->lenght == 0) {
			infoprint("<Netlink::senddata()> tamanho do buffer nao informado\n");
			verifydata(-1);
		}
		
		// se nao houver envio em pacotes de dados
		if (sendpacklen == 0) {

			//envia o tamanho do buffer a ser enviado
			verifydata(send(sockets[CONNECT_SOCKET], (char*)&data->lenght, sizeof(data->lenght), NULL));

			// e envia os dados
			verifydata(send(sockets[CONNECT_SOCKET], (char*)data->data, data->lenght, NULL));
		}
		// se houver
		else if (sendpacklen > 0) {
			// divide a data em pacotes
			DATA_PACK_SPLIT(data, sendpacklen);

			// e envia os dados
			senddata_packed(data);
		}
		// se o valor da variavel Netlink.sendpacklen for um erro
		else {
			infoprint("<Netlink::senddata()> tamanho do pacote de dados negativo Netlink.sendpacklen\n");
			verifydata(-1);
		}
	}

	//envia os dados em pacotes
	void senddata_packed(NETDATA *data) {

		// envia o tamanho total de dados a serem enviados
		verifydata(send(sockets[CONNECT_SOCKET], (char*)&data->lenght, sizeof(data->lenght), NULL));

		// envia os pacotes
		for (; data->loaded < data->packload; data->loaded++) {
			
			//envia o pacote
			if (data->overflowpacklen == 0) verifydata(send(sockets[CONNECT_SOCKET], (char*)data->packdata[data->loaded], sendpacklen, NULL));

			// se ouver data quebrada
			else if (data->overflowpacklen >= 0) {

				// e nao for o ultimo pacote executa normalmente
				if (data->loaded < data->packload) verifydata(send(sockets[CONNECT_SOCKET], (char*)data->packdata[data->loaded], sendpacklen, NULL));

				// se for o ultimo pacote, envia somente o tamanho do pacote restante
				else if (data->loaded == data->packload) verifydata(send(sockets[CONNECT_SOCKET], (char*)data->packdata[data->loaded], data->overflowpacklen, NULL));
			}
		}
	}
};

#endif // WINSYS_H
