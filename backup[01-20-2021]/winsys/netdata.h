#ifndef NETDATA_H
#define NETDATA_H

// para o strlen
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..//debug.h"

// tipos de dados possiveis
constexpr char DATA_TYPE_INT = 'i';
constexpr char DATA_TYPE_CHAR = 'c';
constexpr char DATA_TYPE_FLOAT = 'f';
constexpr char DATA_TYPE_DOUBLE = 'd';
constexpr char DATA_TYPE_STRING = 's';
constexpr char DATA_TYPE_VOID = '0';

struct NETDATA {
private:
	// determina se a variavel data é criada localmente
	bool local = false;
	template <class T> 
	void initdata(T val, char typ) {
		if (local) free(data);
		local = true;
		T* tmp = (T*)malloc(sizeof(T));
		*tmp = val;
		data = (void*)tmp;
		lenght = sizeof(T);
		type = typ;
	}
	
public:

	// a data em si
	void* data = nullptr;
	// o tamanho da data em bytes
	int lenght = 0;
	// a quantidade de pacotes de envio/recebimento
	int packload = 0;
	// quantos pacotes foram enviados/recebidos
	int loaded = 0;
	// determina o tipo de dado
	char type = '0';

	void operator= (const char* val) {
		if (local) free(data);
		local = false;
		data = (void*)val;
		lenght = strlen(val) + 1; // o +1 é para o \n
		type = 's';
	}

	void operator= (const char val) { initdata<char>(val, 'c'); }

	void operator= (double val) { initdata<double>(val, 'd'); }

	void operator= (float val) { initdata<float>(val, 'f'); }

	void operator= (int val) { initdata<int>(val, 'i'); }

	~NETDATA() { if (local) free(data); }
};

// funcao simples para converter data de retorno em valor para uso
template <class T>
inline T DATA_TO(NETDATA* dt) {

	return *((T*)dt->data);
}

// adiciona a data do 2 argumento no 1 argumento, retorna -1 em erro, ou o tamanho final 
int DATA_PACK_MERGE(NETDATA* dt, NETDATA* packdt, int packlen) {
	char* tmp = (char*) realloc(dt->data, packdt->loaded * packlen);
	if (!tmp) {
		debugprint("memoria insuficiente para concluir a operacao");
		return -1;
	}

	char* pktmp =(char*) packdt->data;

	for (int i = 0, last = packdt->loaded - 1 * packlen; i < packlen; i++) tmp[last + i] = pktmp[i];

	dt->data =(void*) tmp;
	return packdt->loaded * packlen;
}



#endif // !NETDATA_H
