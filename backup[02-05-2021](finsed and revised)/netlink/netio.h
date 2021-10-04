#ifndef WINSTREAM_H
#define WINSTREAM_H

#include "netlink.h"

// recebe a data, em pacotes se for determinado valor != 0 para recivepacklen
	// recebe como argumento uma estrutura do tipo NETDATA, para manipulacao
void Netlink::recvdata(NETDATA* data, int ID) {

	// pega o socket correspondete ao ID
	NETSOCKET *sock = getsocket(ID, sockethead);

	// verifica se nao foi inserido buffer nulo
	if (data == nullptr || data->data == nullptr) {
		infoprint("<Netlink::senddata()> buffer nulo inserido como argumento\n");
		verifydata(this, -1);
	}

	// verifica se a um valor de tamanho dos dados
	if (data->lenght == 0) {
		infoprint("<Netlink::senddata()> tamanho do buffer nao informado\n");
		verifydata(this, -1);
	}

	// se o tamanho dos pacotes de dados nao forem determinados receba tudo de uma vez
	if (sock->datapacklen == 0) {

		// primeiro recebe o tamanho da data a ser alocada, em *bytes*
		verifydata(this, recv(sock->socket, (char*)&data->lenght, sizeof(data->lenght), 0));

		//aloca um buffer para sustentar isso
		data->data = realloc(data->data, data->lenght);

		// e recebe os dados
		verifydata(this, recv(sock->socket, (char*)data->data, data->lenght, 0));

	}
	// se ouver envio em pacotes de dados
	else if (sock->datapacklen > 0) {

		// recebe os dados
		do recvdata_packed(data, ID); while (data->loaded < data->packload);

		// e os insere na data
		data_pack_merge_all(data, sock->datapacklen);
	}
	// se ouver um erro na variavel Netlink.sendpacklen encerre
	else {
		infoprint("<Netlink::senddata()> tamanho do pacote de dados negativo Netlink.sendpacklen\n");
		verifydata(this, -1);
	}

}

// funcao semelhante a recv, mas de uso esclusivo a dados enviados em pacotes
// recebe como argumento uma estrutura do tipo NETDATA
// assim podendo tratar separadamente os pacotes recebidos
// os dados sao inseridos em netdata->packdata
// a funcao deve ser chamada a cada recebimento de dados
// os dados recebidos a cada loop ficam armazenados em NETDATA.packdata[NETDATA.loaded - 1];
void Netlink::recvdata_packed(NETDATA* data, int ID) {

	NETSOCKET* sock = getsocket(ID, sockethead);

	// recebe os pacotes ate o fim, se haver terminado, ou for igual a 0. continua.
	if (data->loaded < data->packload) {

		//recebe o pacote
		if (data->overflowpacklen == 0) verifydata(this, recv(sock->socket, (char*)data->packdata[data->loaded], sock->datapacklen, 0));

		// se ouver data quebrada
		else if (data->overflowpacklen >= 0) {

			// e nao for o ultimo pacote executa normalmente
			if (data->loaded < data->packload) verifydata(this, recv(sock->socket, (char*)data->packdata[data->loaded], sock->datapacklen, 0));

			// se for o ultimo, recebe somente o tamanho do pacote restante
			else if (data->loaded == data->packload) verifydata(this, recv(sock->socket, (char*)data->packdata[data->loaded], data->overflowpacklen, 0));
		}

		// aumenta a quantidade de pacotes recebidos
		data->loaded++;

		// encerra a funcao
		return;
	}

	// se nao ouver envio em pacotes, nao faz sentido usar essa funcao, encerra-a e retorna um aviso
	else if (sock->datapacklen == 0) {
		debugerror("nao a uso de pacotes de dados por parte da conexao");
		return;
	}

	// se nada foi recebido ainda
	else if (data->loaded == data->packload != 0) {

		// recebe o tamanho da data a ser recebida
		verifydata(this, recv(sock->socket, (char*)&data->lenght, sizeof(data->lenght), 0));

		//alloca os pacotes, para serem recebidos e pre inicia os dados para alocacao
		data_pack_clean(data, sock->datapacklen);

		// e refaz a chamada a funcao para receber o 1 pacote
		recvdata_packed(data, ID);
	}

	// se a operacao foi concluida limpe o buffer temporario
	else if (data->loaded == data->packload == 0) {
		data_pack_clean(data, sendpacklen);
		debugerror("recebimento de dados por pacotes finalizado");
	}
}

// envia dados a conexao
void Netlink::senddata(NETDATA* data, int ID) {
	
	NETSOCKET* sock = getsocket(ID, sockethead);

	// verifica se nao foi inserido buffer nulo
	if (data == nullptr || data->data == nullptr) {
		infoprint("<Netlink::senddata()> buffer nulo inserido como argumento\n");
		verifydata(this, -1);
	}

	// verifica se a um valor de tamanho dos dados
	if (data->lenght == 0) {
		infoprint("<Netlink::senddata()> tamanho do buffer nao informado\n");
		verifydata(this, -1);
	}

	// se nao houver envio em pacotes de dados
	if (sendpacklen == 0) {

		//envia o tamanho do buffer a ser enviado
		verifydata(this, send(sock->socket, (char*)&data->lenght, sizeof(data->lenght), 0));

		// e envia os dados
		verifydata(this, send(sock->socket, (char*)data->data, data->lenght, 0));
	}
	// se houver
	else if (sendpacklen > 0) {
		// divide a data em pacotes
		data_pack_split(data, sendpacklen);

		// e envia os dados
		senddata_packed(data, ID);

	}
	// se o valor da variavel Netlink.sendpacklen for um erro
	else {
		infoprint("<Netlink::senddata()> tamanho do pacote de dados negativo Netlink.sendpacklen\n");
		verifydata(this, -1);
	}
}

//envia os dados em pacotes
void Netlink::senddata_packed(NETDATA* data, int ID) {

	if (data->loaded > 0) {
		infoprint("<Netlink::senddata_packed()> data inserida ja foi utilizada, use DATA_CLEAR antes de reutilizar a data\n");
		verifydata(this, -1);
	}

	NETSOCKET* sock = getsocket(ID, sockethead);

	// envia o tamanho total de dados a serem enviados
	verifydata(this, send(sock->socket, (char*)&data->lenght, sizeof(data->lenght), 0));

	// envia os pacotes
	for (; data->loaded < data->packload; data->loaded++) {
		
		//envia o pacote
		if (data->overflowpacklen == 0) verifydata(this, send(sock->socket, (char*)data->packdata[data->loaded], sendpacklen, 0));

		// se ouver data quebrada
		else if (data->overflowpacklen >= 0) {

			// e nao for o ultimo pacote executa normalmente
			if (data->loaded < data->packload) verifydata(this, send(sock->socket, (char*)data->packdata[data->loaded], sendpacklen, 0));

			// se for o ultimo pacote, envia somente o tamanho do pacote restante
			else if (data->loaded == data->packload) verifydata(this, send(sock->socket, (char*)data->packdata[data->loaded], data->overflowpacklen, 0));
		}
	}

	// e limpa a data ao terminar
	data_pack_clean(data, sendpacklen);
}

#endif // WINSTREAM_H