#ifndef WINSTREAM_H
#define WINSTREAM_H

#include "netlink.h"

// recebe a data, em pacotes se for determinado valor != 0 para recivepacklen
	// recebe como argumento uma estrutura do tipo NETDATA, para manipulacao
int Netlink::recvdata(NETDATA* data, int ID) {

	// pega o socket correspondete ao ID
	NETSOCKET* sock = getsocket(ID, sockethead);

	if (!data) {                 debugerror(-3, "[erro] em <recvdata()> <NETDATA* data> argumento nulo"); return -3; }
	if (!data->data) {           debugerror(-3, "[erro] em <recvdata()> <NETDATA* data::data> argumento nulo"); return -3; }
	if (data->lenght <= 0) {     debugerror(-4, "[erro] em <recvdata()> <int NETDATA* data::lenght> argumento invalido"); return -3; }
	if (data->loaded < 0) {      debugerror(-4, "[erro] em <recvdata()> <int NETDATA* data::loaded> argumento invalido"); return -3; }
	if (data->packload < 0) {    debugerror(-4, "[erro] em <recvdata()> <int NETDATA* data::packload> argumento invalido"); return -3; }
	if (sock->datapacklen < 0) { debugerror(-4, "[erro] em <recvdata()> <int NETSOCKET sock::datapacklen> argumento invalido"); return -3; }
	if (sock->socket < 0) {      debugerror(-4, "[erro] em <recvdata()> <int NETSOCKET sock::socket> argumento invalido"); return -3; }

	// se o tamanho dos pacotes de dados nao forem determinados receba tudo de uma vez
	if (sock->datapacklen == 0) {

		// primeiro recebe o tamanho da data a ser alocada, em *bytes*
		int r = verify(recv(sock->socket, (char*)&data->lenght, sizeof(data->lenght), 0)); if (r < 0) return r;

		//aloca um buffer para sustentar isso
		data->data = realloc(data->data, data->lenght);

		// e recebe os dados
		r = verify(recv(sock->socket, (char*)data->data, data->lenght, 0)); if (r < 0) return r;

	}
	// se ouver envio em pacotes de dados
	else if (sock->datapacklen > 0) {

		// recebe os dados
		do recvdata_packed(data, ID); while (data->loaded < data->packload);

		// e os insere na data
		data_pack_merge_all(data, sock->datapacklen);
	}

	return 0;
}

// funcao semelhante a recv, mas de uso esclusivo a dados enviados em pacotes
// recebe como argumento uma estrutura do tipo NETDATA
// assim podendo tratar separadamente os pacotes recebidos
// os dados sao inseridos em netdata->packdata
// a funcao deve ser chamada a cada recebimento de dados
// os dados recebidos a cada loop ficam armazenados em NETDATA.packdata[NETDATA.loaded - 1];
int Netlink::recvdata_packed(NETDATA* data, int ID) {

	NETSOCKET* sock = getsocket(ID, sockethead);

	if (!data) {                         debugerror(-3, "[erro] em <recvdata()> <NETDATA* data> argumento nulo"); return -3; }
	if (data->loaded < 0) { debugerror(-4, "[erro] em <recvdata()> <int NETDATA* data::loaded> argumento invalido"); return -3; }
	if (data->lenght <= 0) {             debugerror(-4, "[erro] em <recvdata()> <int NETDATA* data::lenght> argumento invalido"); return -3; }
	if (data->packload < 0) {            debugerror(-4, "[erro] em <recvdata()> <int NETDATA* data::packload> argumento invalido"); return -3; }
	if (data->overflowpacklen < 0) {     debugerror(-4, "[erro] em <recvdata()> <int NETDATA* data::overflowpacklen> argumento invalido"); return -3; }
	if (sock->datapacklen < 0) {         debugerror(-4, "[erro] em <recvdata()> <int NETSOCKET sock::datapacklen> argumento invalido"); return -3; }
	if (sock->socket < 0) {              debugerror(-4, "[erro] em <recvdata()> <int NETSOCKET sock::socket> argumento invalido"); return -3; }

	// recebe os pacotes ate o fim, se haver terminado, ou for igual a 0. continua.
	if (data->loaded < data->packload) {
		if (!data->packdata) { debugerror(-3, "[erro] em <recvdata()> <NETDATA* data::packdata> argumento nulo"); return -3; }
		if (!data->packdata[data->loaded]) { debugerror(-3, "[erro] em <recvdata()> <NETDATA* data::packdata[%d]> argumento nulo", data->loaded); return -3; }

		//recebe o pacote
		if (data->overflowpacklen == 0) { int r = verify(recv(sock->socket, (char*)data->packdata[data->loaded], sock->datapacklen, 0)); if (r < 0) return r; }

		// se ouver data quebrada
		else if (data->overflowpacklen >= 0) {

			// e nao for o ultimo pacote executa normalmente
			if (data->loaded < data->packload) { int r = verify(recv(sock->socket, (char*)data->packdata[data->loaded], sock->datapacklen, 0)); if (r < 0) return r; }

			// se for o ultimo, recebe somente o tamanho do pacote restante
			else if (data->loaded == data->packload) { int r = verify(recv(sock->socket, (char*)data->packdata[data->loaded], data->overflowpacklen, 0)); if (r < 0) return r; }
		}

		

		// aumenta a quantidade de pacotes recebidos
		data->loaded++;

		// retorna quanto falta
		return data->packload - data->loaded;
	}

	// se nada foi recebido ainda
	else if (data->loaded == data->packload != 0) {

		// recebe o tamanho da data a ser recebida
		int r = verify(recv(sock->socket, (char*)&data->lenght, sizeof(data->lenght), 0)); if (r < 0) return r;

		//alloca os pacotes, para serem recebidos e pre inicia os dados para alocacao
		r = data_pack_clean(data, sock->datapacklen);  if (r < 0) return r;

		// e refaz a chamada a funcao para receber o 1 pacote
		r = recvdata_packed(data, ID); if (r < 0) return r;
	}

	// se a operacao foi concluida limpe o buffer temporario
	else if (data->loaded == data->packload == 0) {
		int r = data_pack_clean(data, sock->datapacklen); if (r < 0) return r;
		logprint("recebimento de dados por pacotes finalizado");
	}

	return 0;
}

// envia dados a conexao
int Netlink::senddata(NETDATA* data, int ID) {
	
	NETSOCKET* sock = getsocket(ID, sockethead);

	if (!data) { debugerror(-3, "[erro] em <recvdata()> <NETDATA* data> argumento nulo"); return -3; }
	if (!data->data) { debugerror(-3, "[erro] em <recvdata()> <NETDATA* data::data> argumento nulo"); return -3; }
	if (data->lenght <= 0) { debugerror(-4, "[erro] em <recvdata()> <int NETDATA* data::lenght> argumento invalido"); return -3; }
	if (sendpacklen < 0) { debugerror(-4, "[erro] em <recvdata()> <int Netlink::sendpacklen> argumento invalido"); return -3; }
	if (sock->socket < 0) { debugerror(-4, "[erro] em <recvdata()> <int NETSOCKET sock::socket> argumento invalido"); return -3; }

	// se nao houver envio em pacotes de dados
	if (sendpacklen == 0) {
		//envia o tamanho do buffer a ser enviado
		int r = verify(send(sock->socket, (char*)&data->lenght, sizeof(data->lenght), 0)); if (r < 0) return r;

		// e envia os dados
		r = verify(send(sock->socket, (char*)data->data, data->lenght, 0)); if (r < 0) return r;
	}
	// se houver
	else if (sendpacklen > 0) {
		// divide a data em pacotes
		int r = data_pack_split(data, sendpacklen); if (r < 0) return r;

		// e envia os dados
		r = senddata_packed(data, ID); if (r < 0) return r;
	}
	return 0;
}

//envia os dados em pacotes
int Netlink::senddata_packed(NETDATA* data, int ID) {
	NETSOCKET* sock = getsocket(ID, sockethead);

	if (!data) { debugerror(-3, "[erro] em <recvdata()> <NETDATA* data> argumento nulo"); return -3; }
	if (data->loaded < 0) { debugerror(-4, "[erro] em <recvdata()> <int NETDATA* data::loaded> argumento invalido"); return -3; }
	if (!data->packdata) { debugerror(-3, "[erro] em <recvdata()> <NETDATA* data::packdata> argumento nulo"); return -3; }
	if (!data->packdata[data->loaded]) { debugerror(-3, "[erro] em <recvdata()> <NETDATA* data::packdata[%d]> argumento nulo", data->loaded); return -3; }
	if (data->lenght <= 0) { debugerror(-4, "[erro] em <recvdata()> <int NETDATA* data::lenght> argumento invalido"); return -3; }
	if (data->packload < 0) { debugerror(-4, "[erro] em <recvdata()> <int NETDATA* data::packload> argumento invalido"); return -3; }
	if (data->overflowpacklen < 0) { debugerror(-4, "[erro] em <recvdata()> <int NETDATA* data::overflowpacklen> argumento invalido"); return -3; }
	if (sendpacklen < 0) { debugerror(-4, "[erro] em <recvdata()> <int Netlink::sendpacklen> argumento invalido"); return -3; }
	if (sock->socket < 0) { debugerror(-4, "[erro] em <recvdata()> <int NETSOCKET sock::socket> argumento invalido"); return -3; }
	
	// envia o tamanho total de dados a serem enviados
	int r = verify(send(sock->socket, (char*)&data->lenght, sizeof(data->lenght), 0));

	// envia os pacotes
	for (; data->loaded < data->packload; data->loaded++) {
		
		//envia o pacote
		if (data->overflowpacklen == 0) { int r = verify(send(sock->socket, (char*)data->packdata[data->loaded], sendpacklen, 0)); if (r < 0) return r; }

		// se ouver data quebrada
		else if (data->overflowpacklen >= 0) {

			// e nao for o ultimo pacote executa normalmente
			if (data->loaded < data->packload) { int r = verify(send(sock->socket, (char*)data->packdata[data->loaded], sendpacklen, 0)); if (r < 0) return r; }

			// se for o ultimo pacote, envia somente o tamanho do pacote restante
			else if (data->loaded == data->packload) { int r = verify(send(sock->socket, (char*)data->packdata[data->loaded], data->overflowpacklen, 0)); if (r < 0) return r; }
		}
	}

	// e limpa a data ao terminar
	r = data_pack_clean(data, sendpacklen); if (r < 0) return r;
}

#endif // WINSTREAM_H