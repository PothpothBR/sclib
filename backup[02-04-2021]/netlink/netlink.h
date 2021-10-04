#ifndef WINSYS_H
#define WINSYS_H

/*
Cabecalho para inclusao de funcoes para manipular
o envio de dados em redes para compatbilidade universal
entre dispositivos.

Sendo este um nivel de compatbilidade para windows.

estrutura de inclusao de arquivos:

                              + winit +
netdata > netlink > netstream +       + linkinit > netlib
							  + linit +

netdata - para a estrutura de armazenamento de dados

netlink - para a estrutura de conexao

netstream - metodos de Netlink para fluxo de dados

winit - windows like metodos de Netlink para conexao, construtores e destrutores, e otros metodos privados

linit - linux like metodos de Netlink para conexao, construtores e destrutores, e otros metodos privados

linkinit - include para definir os arquivos corretos de inclusao para cada sistema

netlib - arquivo de inclusao para projetos
		  
*/

// fazer esse arquivo universal
// mover todos os metodos e variaveis da estrutura que sao compativeis com o windows para um arquivo separado 
// CRIAR SISTEMA DE ERROS QUE NAO QUEBRE O PROGRAMA E SIM RETORNE-OS PARA O PROGRAMA DECIDIR OQUE FAZER
// CRIAR SISTEMA PARA IDENTIFICAR FALNAS NA CONEXAO, POIS É ERRO FACIL DE OCORRER E FACILMENTE TRATADO

// estrutura netdata
#include "netdata.h"

// macros para definicao do funcionamento do sistema
constexpr bool SERVER_SIDE = true;
constexpr bool CLIENT_SIDE = false;

// macros para definir estado de conexao
constexpr bool SOCK_CONNECTED = true;
constexpr bool SOCK_UNCONNECTED = false;

struct NETSOCKET {

	// armazena os sockets
	int socket = 0;

	// define o estado de conexao
	bool connected = 0;

	// taxa de dados por pacote que o outro lado envia
	int datapacklen = 0;

	// nome asignado para a conexao
	int ID = 0;

	// endereco de ip da conexao
	char IP[14] = { ' ' };

	// porta da conexao
	char port[5] = { ' ' };

	// latencia da conexao
	int ms = 0;

	// para lista encadeada
	NETSOCKET* next = nullptr;
	NETSOCKET* previus = nullptr;

};

struct Netlink {
private:
	// num sei oque e, mas precisa, entao ta ai
	compatbility_level_data;

	// armazena os sockets e informacoes do mesmo
	NETSOCKET *sockethead = nullptr;
	NETSOCKET* socketfoot = nullptr;

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

	// alloca um novo socket ao buffer
	static void addsocket(Netlink* self, int ID) {

		// guarda o antigo topo
		NETSOCKET* oldhead = self->sockethead;

		// aloca o socket no topo da lista encadeada
		self->sockethead =(NETSOCKET*) calloc(1, sizeof(NETSOCKET));

		// verifica erros
		if (!self->sockethead) {
			infoprint("null memory return by realloc\n");
			verify(0);
			shutdown();
		}

		// encadeia o novo socket na lista
		self->sockethead->next = oldhead;

		
		// encadeamento reverso
		if (oldhead) {
			oldhead->previus = self->sockethead;

			// verifica se o socket removido do topo e o pe
			if (oldhead->next == nullptr) {
				self->socketfoot = oldhead;
			}
		}
		//aumenta a contagem da lista
		self->socketslen++;

		// insere a identificacao ao socket
		self->sockethead->ID = ID;
	}

	// retorna um socket objeto completo
	static NETSOCKET* getsocket(Netlink *self, int ID) {
		for (NETSOCKET* ptr = self->sockethead; ptr != nullptr; ptr = ptr->next) {
			if (ptr->ID == ID) return ptr;
		}

		// se nao encontrar quebre o programa
		infoprint("ID informado nao existe");
		verify(true);
	}

	// fecha todos os sockets
	static void closeallsockets(Netlink* self) {
		
		// libera o socket e a NETSOCKET estrutura
		if (self->sockethead)
		for (NETSOCKET* ptr = self->sockethead, *del = ptr; ptr != nullptr; del = ptr) {
			compatbility_level_closesocket(ptr->socket);
			ptr = ptr->next;
			free(del);
		}

		if (self->linktype == SERVER_SIDE) compatbility_level_closesocket(self->listensocket.socket);
	}

	// fecha um unico socket
	static void closeonesocket(Netlink* self, int ID) {
		// pega o socket
		NETSOCKET* unbind = getsocket(self, ID);
	
		// remove-o da lista encadeando o seu anterior no seu proximo
		unbind->previus = unbind->next;

		// fecha a conexao e libera o socket
		compatbility_level_closesocket(unbind->socket);
		free(unbind);
		
	}

	// verifica erro critico e encerra o programa se necessario
	static bool verify(bool cond) {
		if (cond) {
			infoprint("erro coletado: %d\n", compatbility_level_geterror());
			compatbility_level_cleanup();
		}
		return cond;
	}

	// verifica erros ao enviar ou receber
	static int verifydata(Netlink* self, int result) {

		if (result >= 0) return result;
		else {
			debugprint("erro ao enviar ou receber dados");
			verify(true);
			closeallsockets(self);
			shutdown();
		}
	}

	// encerramento simples do sistema
	static void shutdown() {
		debugprint(" <Netlink::shutdown()> encerrando o programa antes do seu fim real");
		getchar();
		getchar();
		// dis getchar pois ironicamente so um nao pausa....
		exit(0);
	}

public:

	// referencia externa para o contrutor e destrutor
	Netlink(bool, int, const char*);
	~Netlink();
	
	// recebe a data, em pacotes se for determinado valor != 0 para recivepacklen
	// recebe como argumento uma estrutura do tipo NETDATA, para manipulacao
	void recvdata(NETDATA*, int);

	// funcao semelhante a recv, mas de uso esclusivo a dados enviados em pacotes
	// recebe como argumento uma estrutura do tipo NETDATA
	// assim podendo tratar separadamente os pacotes recebidos
	// os dados sao inseridos em netdata->packdata
	// a funcao deve ser chamada a cada recebimento de dados
	// os dados recebidos a cada loop ficam armazenados em NETDATA.packdata[NETDATA.loaded - 1];
	void recvdata_packed(NETDATA*, int);

	// envia dados a conexao
	void senddata(NETDATA*, int);

	//envia os dados em pacotes
	void senddata_packed(NETDATA*, int);

	// conecta o sistema a alguem
	// a conexao fica em uma lista encadeada 
	void waitconnection(const char*, const char*);

	// retorna o ultimo socket conectado
	NETSOCKET* lastsocket() { return sockethead; }

	// retorna o id do ultimo socket, sendo possivel modificalo
	int lastsocketid() { return sockethead->ID; }

	// retorna o ip do ultimo socket
	const char* const lastsocketip() { return sockethead->IP; }

	void changesockid(int ID, int newID) { getsocket(this, ID)->ID = newID; }

	NETSOCKET* getsocketbyid(int ID) { return getsocket(this, ID); }
};



#endif // WINSYS_H
