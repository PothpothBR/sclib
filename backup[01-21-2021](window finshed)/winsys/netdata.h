#ifndef NETDATA_H
#define NETDATA_H

// para o strlen
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..//debug.h"

// definido para uso esclusivo em mainpulacao, nao sendo usado para incersao ou coleta dedados
typedef char data_t;

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

	// a data completa em si
	void* data = nullptr;
	// o tamanho da data em bytes
	int lenght = 0;
	// a data dividida em pacotes
	void** packdata = nullptr;
	// tamanho dos pacotes de dados
	int packlen = 0; // nao adicionado ao sistema ainda!!!!!!
	// a quantidade de pacotes de envio/recebimento
	int packload = 0;
	// quantos pacotes foram enviados/recebidos
	int loaded = 0;
	// se ouver pacotes quebrados, esse é o quanto transbordou de data
	int overflowpacklen = 0;
	// determina o tipo de dado
	char type = '0';

	void operator= (const char* val) {
		if (local) free(data);
		local = true;
		lenght = strlen(val) + 1; // o +1 é para o \n
		char* tmp = (char*)calloc(sizeof(char), lenght);
		for (int i = 0; i < lenght; i++) tmp[i] = val[i];
		data = (void*)tmp;
		type = 's';
	}

	void operator= (const char val) { initdata<char>(val, 'c'); }

	void operator= (double val) { initdata<double>(val, 'd'); }

	void operator= (float val) { initdata<float>(val, 'f'); }

	void operator= (int val) { initdata<int>(val, 'i'); }

	~NETDATA() { if (local) free(data); }
};

// aloca um espaco para receber os dados em pacotes e pre inicia os dados nescessarios
int DATA_PACK_CLEAN(NETDATA* dt, int packlen) {

	// a quantidade de pacotes
	dt->packload = dt->lenght / packlen;

	// se sobrar dados adicione um ultimo pacote para alocar
	dt->overflowpacklen = dt->lenght % packlen;
	if (dt->overflowpacklen > 0) dt->packload++;

	// e indica que nada foi carregado
	dt->loaded = 0;

	// cria um buffer para alocar os pacotes
	data_t** tmp = (data_t**)calloc(dt->packload, sizeof(data_t*));


	// aloca os pacotes, ate o penultimo
	for (int i = 0; i < dt->packload - 1; i++) {
		tmp[i] = (data_t*)calloc(packlen, sizeof(data_t));
	}

	// se houver aloca somente a sobra
	if (dt->overflowpacklen > 0) tmp[dt->packload - 1] = (data_t*)calloc(dt->overflowpacklen, sizeof(data_t));
	// se nao ouver sobras aloca normalmete 
	else if (dt->overflowpacklen == 0) tmp[dt->packload - 1] = (data_t*)calloc(packlen, sizeof(data_t));

	//verifica a validade
	if (!tmp) {
		debugprint("memoria insuficiente para concluir a operacao");
		return -1;
	}

	// se ouver dados alocados, remove-os
	if (dt->packdata) {
		for (int i = 0; i < dt->packload; i++) {
			free(dt->packdata[i]);
		}
		free(dt->packdata);
	}

	// e insere a nova data no buffer
	dt->packdata = (void**)tmp;
}

// limpa a data principal e aloca um espaco para receber dados
void DATA_CLEAN(NETDATA* dt, int lenght) {
	if (dt->data) free(dt->data);

	dt->data = calloc(lenght, 1);

	dt->loaded = 0;

	dt->lenght = lenght;
}

// adiciona a data do pacote na posicao especificada por loaded, pois ate ai, a data ja foi carregada na data completa, retorna -1 em erro, ou o tamanho final 
int DATA_PACK_MERGE(NETDATA* dt, int packlen) { 

	if (dt->loaded >= dt->packload) {
		debugprint("data chegou ao final, nao a iteracao");
		return -1;
	}

	// aumenta a data para alocar os dados
	// converte para a alocacao
	data_t* tmp = (data_t*)realloc(dt->data, (dt->loaded + 1) * packlen);

	// e pega o proximo pacote(o qual nao foi carregado) para carregar na data
	data_t* packtmp = (data_t*) dt->packdata[dt->loaded];

	//insere o pacote
	if (dt->overflowpacklen == 0) for (int i = 0; i < packlen; i++) { tmp[dt->loaded * packlen + i] = packtmp[i]; }
	// se ouver data quebrada
	else if (dt->overflowpacklen >= 0) {
		// e nao for o ultimo pacote executa normalmente
		if (dt->loaded < dt->packload) for (int i = 0; i < packlen; i++) { tmp[dt->loaded * packlen + i] = packtmp[i]; }
		// snao, insere somente o tamanho do pacote restante
		else if (dt->loaded == dt->packload) for (int i = 0; i < dt->overflowpacklen; i++) { tmp[dt->loaded * packlen + i] = packtmp[i]; }
	}

	// devolve o buffer a data
	dt->data = (void*) tmp;
	// aumenta o iterator de alocacao
	dt->loaded++;
	//retorna o tamanho total apos alocado
	return dt->loaded * packlen;
}

// junta toda a data recebida, zera o loaded valor
int DATA_PACK_MERGE_ALL(NETDATA* dt, int packlen) { 

	dt->loaded = 0;

	// aumenta a data para alocar os dados
	// converte para a alocacao
	data_t* tmp = (data_t*) realloc(dt->data, dt->packload * packlen);

	// cria uma variavel para alocar a conversao
	data_t* packtmp = nullptr;

	//insere os valores
	for (; dt->loaded < dt->packload; dt->loaded++) {

		// converte os pacotes para carregar na data
		packtmp = (data_t*)dt->packdata[dt->loaded];

		//insere o pacote
		if (dt->overflowpacklen == 0) for (int i = 0; i < packlen; i++) { tmp[(dt->loaded) * packlen + i] = packtmp[i]; }
		// se ouver data quebrada
		else if (dt->overflowpacklen >= 0) {
			// e nao for o ultimo pacote executa normalmente
			if (dt->loaded < dt->packload) for (int i = 0; i < packlen; i++) { tmp[(dt->loaded) * packlen + i] = packtmp[i]; }
			// snao, insere somente o tamanho do pacote restante
			else if (dt->loaded == dt->packload) for (int i = 0; i < dt->overflowpacklen; i++) { tmp[(dt->loaded) * packlen + i] = packtmp[i]; }
		}

	}

	

	// devolve o buffer a data
	dt->data = (void*)tmp;
	return dt->loaded * packlen;
}

int DATA_PACK_SPLIT(NETDATA* dt, int packload) {

	// a quantidade de pacotes
	dt->packload = dt->lenght / packload;
	// se sobrar dados adicione um ultimo pacote para alocar
	dt->overflowpacklen = dt->lenght % packload;
	if (dt->overflowpacklen > 0) dt->packload++;

	// cria um buffer para alocar os pacotes
	data_t** tmp = (data_t**) calloc(dt->packload, sizeof(data_t*));


	// aloca os pacotes, ate o penultimo
	for (int i = 0; i < dt->packload - 1; i++) {
		tmp[i] = (data_t*)calloc(packload, sizeof(data_t));
	}

	// se houver aloca somente a sobra
	if (dt->overflowpacklen > 0) tmp[dt->packload - 1] = (data_t*)calloc(dt->overflowpacklen, sizeof(data_t));
	// se nao ouver sobras aloca normalmete 
	else if (dt->overflowpacklen == 0) tmp[dt->packload - 1] = (data_t*)calloc(packload, sizeof(data_t));
	
	//verifica a validade
	if (!tmp) {
		debugprint("memoria insuficiente para concluir a operacao");
		return -1;
	}

	// converte em data_t para manipulacao
	data_t* dttpm = (data_t*) dt->data;
	
	// insere os pacotes ate o penultimo
	for (int i = 0; i < dt->packload - 1; i++) {
		logprint(" pacote: %i\n", i);

		for (int e = 0; e < packload; e++) {
			
			// [pacote][posicao do byte] = [posicao do bite]
			tmp[i][e] = dttpm[i * packload + e];
			logprint(" pos: %i val: %c\n", e, tmp[i][e]);
			
		}

	}

	// se houver copia somente a sobra
	if (dt->overflowpacklen > 0) {
		logprint(" pacote: %i\n", dt->packload - 1);

		for (int e = 0; e < dt->overflowpacklen; e++) {

			// [pacote][posicao do byte] = [posicao do bite]
			tmp[dt->packload - 1][e] = dttpm[(dt->packload - 1) * packload + e];
			logprint(" pos: %i val: %c\n", e, tmp[dt->packload - 1][e]);

		}
	}
	// se nao ouver sobras copia normalmete 
	else if (dt->overflowpacklen == 0) {
		logprint(" pacote: %i\n", dt->packload - 1);

		for (int e = 0; e < packload; e++) {

			// [pacote][posicao do byte] = [posicao do bite]
			tmp[dt->packload - 1][e] = dttpm[(dt->packload - 1) * packload + e];
			logprint(" pos: %i val: %c\n", e, tmp[dt->packload - 1][e]);

		}
	}
	logprint("\n");

	// e referencia a data cortada
	dt->packdata = (void**)tmp;
}



#endif // !NETDATA_H
