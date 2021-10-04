#ifndef NETDATA_H
#define NETDATA_H

/*
biblioteca para facilitacao do envio e recebimento de dados
usando a estrutura NETDATA e funcoes para sua manipulacao

*/

// biblioteca para tratamento de erros
#include "debug.h"

// definido para uso esclusivo em mainpulacao, nao sendo usado para incersao ou coleta dedados
typedef char data_t;

// tipos de dados possiveis
constexpr char DATA_TYPE_INT = 'i';
constexpr char DATA_TYPE_CHAR = 'c';
constexpr char DATA_TYPE_FLOAT = 'f';
constexpr char DATA_TYPE_DOUBLE = 'd';
constexpr char DATA_TYPE_STRING = 's';
constexpr char DATA_TYPE_VOID = '0';

// estrutura para armazenar e tratar dados de envio ou recebimento
struct NETDATA {
private:

	// determina se a variavel data é criada localmente
	bool local = false;

	// inicia um dado bruto e nao alocado
	template <class T> 
	int initdata(T val, char typ) {
		if (local) free(data);
		local = true;
		T* tmp = (T*)malloc(sizeof(T));

		// se estouro de buffer
		if (!tmp) return -1;

		*tmp = val;
		data = (void*)tmp;
		lenght = sizeof(T);
		type = typ;

		// se nada foi inserido
		if (!val) return 1;
	}
	
public:

	// a data completa em si
	void* data = nullptr;

	// o tamanho da data em bytes
	int lenght = 0;

	// a data dividida em pacotes
	void** packdata = nullptr;

	// a quantidade de pacotes de envio/recebimento
	int packload = 0;

	// quantos pacotes foram enviados/recebidos
	int loaded = 0;

	// se ouver pacotes quebrados, esse é o quanto transbordou de data
	int overflowpacklen = 0;

	// determina o tipo de dado
	char type = '0';

	// para insercao de palavras nao pre-alocadas
	void operator= (const char* val) {
		if (local) free(data);
		local = true;
		lenght = (int)strlen(val) + 1; // o +1 é para o \n
		char* tmp = (char*)calloc(sizeof(char), lenght);

		if (!tmp) debugerror(-1, "[erro] em <NETDATA::operator=()> <const char val> estouro no limite de alocamento de memoria");
		if (!val) debugerror(1,  "[warn] em <NETDATA::operator=()> <const char val> valor de atribuicao nulo");

		for (int i = 0; i < lenght; i++) tmp[i] = val[i];
		data = (void*)tmp;
		type = 's';

		
	}

	// para insercao de caraceres nao pre-alocados
	void operator= (const char val) {
		int result = initdata<char>(val, 'c'); 

		if (result == -1) debugerror(-1, "[erro] em <NETDATA::operator=()> <const char val> estouro no limite de alocamento de memoria");
		if (result == 1)  debugerror(1,  "[warn] em <NETDATA::operator=()> <const char val> valor de atribuicao nulo");
	}
	// para insercao de valores de ponto flutuante de tamanho longo nao pre-alocados
	void operator= (double val) {
		int result = initdata<double>(val, 'd');

		if (result == -1) debugerror(-1, "[erro] em <NETDATA::operator=()> <double val> estouro no limite de alocamento de memoria");
		if (result == 1)  debugerror(1,  "[warn] em <NETDATA::operator=()> <double val> valor de atribuicao nulo");
	}
	// para insercao de valores de ponto flutuantes nao pre-alocados
	void operator= (float val) {
		int result = initdata<float>(val, 'f');

		if (result == -1) debugerror(-1, "[erro] em <NETDATA::operator=()> <float val> estouro no limite de alocamento de memoria");
		if (result == 1)  debugerror(1,  "[warn] em <NETDATA::operator=()> <float val> valor de atribuicao nulo");
	}
	// para insercao de valores inteiros nao pre-alocados
	void operator= (int val) {
		int result = initdata<int>(val, 'i');

		if (result == -1) debugerror(-1, "[erro] em <NETDATA::operator=()> <int val> estouro no limite de alocamento de memoria");
		if (result == 1)  debugerror(1,  "[warn] em <NETDATA::operator=()> <int val> valor de atribuicao nulo");
	}

	// construtor vazio
	NETDATA() {};

	// para insercao de palavras nao pre-alocadas
	NETDATA (const char* val) {
		operator=(val);
	}

	// para insercao de caraceres nao pre-alocados
	NETDATA(const char val) {
		operator=(val);
	}

	// para insercao de valores de ponto flutuante de tamanho longo nao pre-alocados
	NETDATA(double val) {
		operator=(val);
	}

	// para insercao de valores de ponto flutuantes nao pre-alocados
	NETDATA(float val) {
		operator=(val);
	}

	// para insercao de valores inteiros nao pre-alocados
	NETDATA(int val) {
		operator=(val);
	}

	// destrutor para liberar a data
	~NETDATA() {
		// libera os dados
		if (local) { 
			if (!data) debugerror(-2, "[erro] em <~NETDATA> <void NETDATA::data> buffer nulo, erro ao liberar memoria");
			free(data); 
		}

		// libera os dados cortados
		if (packdata) {
			for (int i = 0; i < packload; i++) {
				if (!packdata[i]) debugerror(-2, "[erro] em <~NETDATA> <void NETDATA::packdata[%d]> buffer nulo, erro ao liberar memoria", i);
				free(packdata[i]);
			}
			free(packdata);
		}
	}
};

#define DATA_TO(type) *(type*)

//inicia a estrutura para receber uma quantidade de dados especifico e de tipo especifico
template <class T>
int data_init(NETDATA* dt, char typ = '0', T val = T(0), int lenght = 0) {

	if (!dt) {  debugerror(-3, "[error] em <data_init()> <NETDATA* dt> argumento nulo"); return -3; }
	if (!typ) { debugerror(-3, "[error] em <data_init()> <char typ> argumento nulo"); return -3; }
	if (!val)   debugerror(2,  "[warn] em <data_init()> <T val> argumento nao definido, nao inserindo valor");
	if (!val)   debugerror(2,  "[warn] em <data_init()> <int lenght> argumento nao definido, gerando tamanho automaticamente");

	T* tmp = (T*)calloc(1, (lenght) ? lenght : sizeof(T));

	if (!tmp){  debugerror(-1, "[error] em <data_init()> estouro no limite de alocamento de memoria"); return -1; }

	*tmp = val;
	dt->data = (void*)tmp;
	dt->lenght = sizeof(T);
	dt->type = typ;
	return 0;
}

//encerra a estrutura iniciada por data_init
int data_shutdown(NETDATA* dt) {
	if (!dt) { debugerror(-3, "[erro] em <data_shutdown()> <NETDATA* dt> argumento nulo"); return -3; }
	if (!dt->data) { debugerror(-2, "[erro] em <data_shutdown()> <void NETDATA* dt::data> buffer nulo, erro ao liberar memoria"); return -2; }

	free(dt->data);
	dt->data = nullptr;
	dt->lenght = 0;
	dt->type = '0';
	return 0;
}

// aloca um espaco para receber os dados em pacotes e pre inicia os dados nescessarios
int data_pack_clean(NETDATA* dt, int packlen) {

	if (!dt) { debugerror(-3, "[erro] em <data_pack_clean()> <NETDATA* dt> argumento nulo"); return -3; }
	if (dt->lenght <= 0) { debugerror(-4, "[erro] em <data_pack_clean()> <int NETDATA* dt::lenght> argumento invalido"); return -3; }
	if (packlen <= 0) { debugerror(-4, "[erro] em <data_pack_clean()> <int packlen> argumento invalido"); return -4; }

	// a quantidade de pacotes
	dt->packload = dt->lenght / packlen;

	// se sobrar dados adicione um ultimo pacote para alocar
	dt->overflowpacklen = dt->lenght % packlen;
	if (dt->overflowpacklen > 0) dt->packload++;

	// e indica que nada foi carregado
	dt->loaded = 0;

	// cria um buffer para alocar os pacotes
	data_t** tmp = (data_t**)calloc(dt->packload, sizeof(data_t*));
	if (!tmp) { debugerror(-1, "[error] em <data_pack_clean()> estouro no limite de alocamento de memoria"); return -1; }

	// aloca os pacotes, ate o penultimo
	for (int i = 0; i < dt->packload - 1; i++) {
		tmp[i] = (data_t*)calloc(packlen, sizeof(data_t));

		if (!tmp[i]) { debugerror(-1, "[error] em <data_pack_clean()> estouro no limite de alocamento de memoria"); return -1; }
	}

	// se houver aloca somente a sobra
	if (dt->overflowpacklen > 0) tmp[dt->packload - 1] = (data_t*)calloc(dt->overflowpacklen, sizeof(data_t));

	// se nao ouver sobras aloca normalmete 
	else if (dt->overflowpacklen == 0) tmp[dt->packload - 1] = (data_t*)calloc(packlen, sizeof(data_t));

	// se ouver dados alocados, remove-os
	if (dt->packdata) {
		for (int i = 0; i < dt->packload; i++) {
			if (!dt->packdata[i]) { debugerror(-2, "[erro] em <data_pack_clean()> <void NETDATA::packdata[%d]> buffer nulo, erro ao liberar memoria", i); return -2; }
			free(dt->packdata[i]);
		}
		free(dt->packdata);
	}

	// e insere a nova data no buffer
	dt->packdata = (void**)tmp;
	return 0;
}

// limpa a data principal ja alocada anterirmente e zera o envio
int data_clean(NETDATA* dt) {
	if (!dt) { debugerror(-3, "[erro] em <data_clean()> <NETDATA* dt> argumento nulo"); return -3; }
	if (!dt->data) { debugerror(-2, "[erro] em <data_clean()> <void NETDATA* dt::data> buffer nulo, erro ao limpar memoria"); return -2; }
	if (dt->lenght <= 0) { debugerror(-4, "[erro] em <data_clean()> <int NETDATA* dt::lenght> argumento invalido"); return -3; }

	// limpa o buffer
	memset(dt->data, 0, dt->lenght);
	// inicia em 0
	dt->loaded = 0;
	return 0;
}

// limpa totalmente a estrutura
int netdata_clean(NETDATA* dt) {
	if (!dt) { debugerror(-3, "[erro] em <netdata_clean()> <NETDATA* dt> argumento nulo"); return -3; }
	if (!dt->data) { debugerror(-2, "[erro] em <netdata_clean()> <void NETDATA* dt::data> buffer nulo, erro ao liberar memoria"); return -2; }

	// libera os dados
	if (dt->data) free(dt->data);

	// libera os dados cortados
	if (dt->packdata) {
		for (int i = 0; i < dt->packload; i++) {
			if (!dt->packdata[i]) { debugerror(-2, "[erro] em <netdata_clean()> <void NETDATA::packdata[%d]> buffer nulo, erro ao liberar memoria", i); return -2; }
			free(dt->packdata[i]);
		}
		free(dt->packdata);
	}

	dt->loaded = 0;
	dt->loaded = 0;
	dt->overflowpacklen = 0;
	dt->packload = 0;
	dt->type = '0';
	return 0;
}

// adiciona a data do pacote na posicao especificada por loaded, pois ate ai, a data ja foi carregada na data completa, retorna erro, ou o tamanho final 
int data_pack_merge(NETDATA* dt, int packlen) { 
	if (packlen <= 0) {              debugerror(-4, "[erro] em <data_pack_merge()> <int packlen> argumento invalido"); return -3; }
	if (!dt) {                       debugerror(-3, "[erro] em <data_pack_merge()> <NETDATA* dt> argumento nulo"); return -3; }
	if (dt->packload <= 0) {         debugerror(-4, "[erro] em <data_pack_merge()> <int NETDATA* dt::packload> argumento invalido"); return -3; }
	if (dt->overflowpacklen < 0) {   debugerror(-4, "[erro] em <data_pack_merge()> <int NETDATA* dt::overflowpacklen> argumento invalido"); return -3; }
	if (dt->loaded < 0) {            debugerror(-4, "[erro] em <data_pack_merge()> <int NETDATA* dt::loaded> argumento invalido"); return -3; }
	if (!dt->packdata) {             debugerror(-2, "[erro] em <data_pack_merge()> <void NETDATA* dt::packdata> buffer nulo, erro ao alocar memoria"); return -2; }
	if (!dt->packdata[dt->loaded]) { debugerror(-2, "[erro] em <data_pack_merge()> <void NETDATA* dt::packdata[%d]> buffer nulo, erro ao alocar memoria", dt->loaded); return -2; }

	// se o processo ja foi terminado somente retorne
	if (dt->loaded >= dt->packload) { debugerror(3, "[warn] em <data_pack_merge()> <int NETDATA* dt::loaded> processo finalizado"); return 0; }

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
int data_pack_merge_all(NETDATA* dt, int packlen) { 
	if (packlen <= 0) {            debugerror(-4, "[erro] em <data_pack_merge_all()> <int packlen> argumento invalido"); return -3; }
	if (!dt) {                     debugerror(-3, "[erro] em <data_pack_merge_all()> <NETDATA* dt> argumento nulo"); return -3; }
	if (dt->packload <= 0) {       debugerror(-4, "[erro] em <data_pack_merge_all()> <int NETDATA* dt::packload> argumento invalido"); return -3; }
	if (dt->overflowpacklen < 0) { debugerror(-4, "[erro] em <data_pack_merge_all()> <int NETDATA* dt::overflowpacklen> argumento invalido"); return -3; }
	if (!dt->packdata) {           debugerror(-2, "[erro] em <data_pack_merge_all()> <void NETDATA* dt::dt->packdata> buffer nulo, erro ao alocar memoria"); return -2; }

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
		if (!dt->packdata[dt->loaded]) { debugerror(-2, "[erro] em <data_pack_merge_all()> <void NETDATA* dt::dt->packdata[%d]> buffer nulo, erro ao alocar memoria", dt->loaded); return -2; }

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

// separa a data em pacotes para o envio
int data_pack_split(NETDATA* dt, int packlen) {
	if (packlen <= 0) {    debugerror(-4, "[erro] em <data_pack_split()> <int packlen> argumento invalido"); return -3; }
	if (!dt) {             debugerror(-3, "[erro] em <data_pack_split()> <NETDATA* dt> argumento nulo"); return -3; }
	if (!dt->data) {       debugerror(-2, "[erro] em <data_pack_split()> <void NETDATA* dt::data> buffer nulo, erro ao modificar memoria"); return -2; }
	if (dt->lenght <= 0) { debugerror(-4, "[erro] em <data_pack_split()> <int NETDATA* dt::lenght> argumento invalido"); return -3; }

	// a quantidade de pacotes
	dt->packload = dt->lenght / packlen;
	// se sobrar dados adicione um ultimo pacote para alocar
	dt->overflowpacklen = dt->lenght % packlen;
	if (dt->overflowpacklen > 0) dt->packload++;

	// cria um buffer para alocar os pacotes
	data_t** tmp = (data_t**) calloc(dt->packload, sizeof(data_t*));

	if (!tmp) { debugerror(-1, "[erro] em <data_pack_split()> <NETDATA* dt::packdata> estouro no limite de alocamento de memoria"); return -1; }

	// aloca os pacotes, ate o penultimo
	for (int i = 0; i < dt->packload - 1; i++) {
		tmp[i] = (data_t*)calloc(packlen, sizeof(data_t));
		if (!tmp[i]) { debugerror(-1, "[erro] em <data_pack_split()> <NETDATA* dt::packdata[%d]> estouro no limite de alocamento de memoria", i); return -1; }
	}

	// se houver aloca somente a sobra
	if (dt->overflowpacklen > 0) tmp[dt->packload - 1] = (data_t*)calloc(dt->overflowpacklen, sizeof(data_t));
	// se nao ouver sobras aloca normalmete 
	else if (dt->overflowpacklen == 0) tmp[dt->packload - 1] = (data_t*)calloc(packlen, sizeof(data_t));

	// converte em data_t para manipulacao
	data_t* dttpm = (data_t*) dt->data;

	// insere os pacotes ate o penultimo
	for (int i = 0; i < dt->packload - 1; i++) {
		logprint(" pacote: %i\n", i);
		for (int e = 0; e < packlen; e++) {
			// [pacote][posicao do byte] = [posicao do bite]
			tmp[i][e] = dttpm[i * packlen + e];
			logprint(" pos: %i val: %c\n", e, tmp[i][e]);
		}
	}

	// se houver copia somente a sobra
	if (dt->overflowpacklen > 0) {
		logprint(" pacote: %i\n", dt->packload - 1);
		for (int e = 0; e < dt->overflowpacklen; e++) {
			// [pacote][posicao do byte] = [posicao do bite]
			tmp[dt->packload - 1][e] = dttpm[(dt->packload - 1) * packlen + e];
			logprint(" pos: %i val: %c\n", e, tmp[dt->packload - 1][e]);
		}
	}
	// se nao ouver sobras copia normalmete 
	else if (dt->overflowpacklen == 0) {
		logprint(" pacote: %i\n", dt->packload - 1);
		for (int e = 0; e < packlen; e++) {
			// [pacote][posicao do byte] = [posicao do bite]
			tmp[dt->packload - 1][e] = dttpm[(dt->packload - 1) * packlen + e];
			logprint(" pos: %i val: %c\n", e, tmp[dt->packload - 1][e]);
		}
	}
	logprint("\n");

	// e referencia a data cortada
	dt->packdata = (void**)tmp;
	return 0;
} 

#endif // NETDATA_H
