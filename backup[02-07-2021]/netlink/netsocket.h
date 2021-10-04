#ifndef NETSOCKET_H
#define NETSOCKET_H

#include "netdata.h"

// macros para definir estado de conexao
constexpr bool SOCK_CONNECTED = true;
constexpr bool SOCK_UNCONNECTED = false;

struct NETSOCKET {

	// armazena os sockets
	int socket = ~(0);

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

	// para lista encadeada - proximo socket
	NETSOCKET* next = nullptr;
	// socket anterior
	NETSOCKET* previus = nullptr;
};

// alloca um novo socket ao ponteiro de sockets, se foot = 0 nao sera feito encadeamento reverso
int addsocket(int ID, NETSOCKET** head, NETSOCKET** foot, int* lenght) {
	if (!lenght) { debugerror(-3, "[erro] em <addsocket()> <int* lenght> argumento nulo"); return -3; }
	if (*lenght < 0) { debugerror(-4, "[erro] em <addsocket()> <int* lenght> argumento invalido"); return -3; }

	// guarda o antigo topo
	NETSOCKET* oldhead = *head;

	// aloca o socket no topo da lista encadeada
	*head = (NETSOCKET*)calloc(1, sizeof(NETSOCKET));

	if (!(*head)) { debugerror(-1, "[erro] em <addsocket()> estouro no limite de alocamento de memoria"); return -1; }

	// encadeia o novo socket na lista
	(*head)->next = oldhead;
	
	// encadeamento reverso
	if (oldhead && foot) {
		oldhead->previus = *head;

		// verifica se o socket removido do topo e o pe
		if (oldhead->next == nullptr) {
			*foot = oldhead;
		}
	}
	//aumenta a contagem da lista
	*lenght +=1; // ver isso

	// insere a identificacao ao socket
	(*head)->ID = ID;
	return 0;
}

// retorna um socket objeto completo
NETSOCKET* getsocket(int ID, NETSOCKET* head) {
	if (!head) { debugerror(-3, "[erro] em <getsocket()> <NETSOCKET* head> argumento nulo"); return (NETSOCKET*)-3; }

	for (NETSOCKET* ptr = head; ptr != nullptr; ptr = ptr->next) if (ptr->ID == ID) return ptr;

	debugerror(4, "[warn] em <getsocket()> <int ID> nao encontrado"); return (NETSOCKET*)0;
}

// fecha todos os sockets
int closeallsockets(NETSOCKET* head) {
	if (!head) { debugerror(-3, "[erro] em <closeallsockets()> <NETSOCKET* head> argumento nulo"); return -3; }

	// libera a NETSOCKET estrutura encadeada
	if (head)
		for (NETSOCKET* ptr = head, *del = ptr; ptr != nullptr; del = ptr) {
			compatibility_level_closesocket(ptr->socket);
			ptr = ptr->next;
			free(del);
		}
	return 0;
}

// fecha um unico socket
int closeonesocket(int ID, NETSOCKET** head) {
	if (!(*head)) { debugerror(-3, "[erro] em <closeallsockets()> <NETSOCKET* head> argumento nulo"); return -3; }

	// pega o socket
	NETSOCKET* unbind = getsocket(ID, *head);

	if (!unbind){ debugerror(-5, "[erro] em <closeonesocket()> <int ID> nao encontrado"); return -5; }

	// remove-o da lista encadeando o seu anterior no seu proximo,se for o primeiro encadeie o proximo no head
	(unbind == *head)? *head = unbind->next : unbind->previus = unbind->next;

	// fecha a conexao e libera o socket
	compatibility_level_closesocket(unbind->socket);
	free(unbind);
	return 0;
}

#endif //NETSOCKET_H