#ifndef DEBUG_H
#define DEBUG_H

// va_list - va_start - ##__VA_ARGS__
#include <stdio.h>
#include <stdarg.h>

// se, por padrao nada for definido define nivel minimo de informacao, niveis:
/*
 * INFO_LEVEL_MIN somente mensagens padrao do servidor
 * INFO_LEVEL_MED debug ativado
 * INFO_LEVEL_MAX informacoes sobre toda a execussao do programa
 * INFO_LEVEL_ZERO nenhuma informacao, console limpo
*/

#if !defined(INFO_LEVEL_MIN) && !defined(INFO_LEVEL_MED) && !defined(INFO_LEVEL_MAX) && !defined(INFO_LEVEL_ZERO)
#define INFO_LEVEL_MIN
#endif

#if defined(INFO_LEVEL_ZERO)
// informacoes do estado atual e processo de funcionamento do server
#define infoprint(vals, ...)

// informacoes de exito ou falha na execussao de comandos
#define debugprint(vals, ...)

// informacoes de dados, variaveis, estados, etc
#define logprint(vals, ...)

#elif defined(INFO_LEVEL_MIN)

// informacoes do estado atual e processo de funcionamento do server
// informacoes do estado atual e processo de funcionamento do server
void infoprint(const char* vals, ...) {

	char buffer[256];

	va_list va;
	va_start(va, vals);

	compatbility_level_vsprintf(buffer, vals, va);

	printf(" --> %s", buffer);
}

// informacoes de exito ou falha na execussao de comandos
#define debugprint(vals, ...)

// informacoes de dados, variaveis, estados, etc
#define logprint(vals, ...)

#elif defined(INFO_LEVEL_MED)

// informacoes do estado atual e processo de funcionamento do server
void infoprint(const char* vals, ...) {

	char buffer[256];

	va_list va;
	va_start(va, vals);

	vsprintf_s(buffer, vals, va);

	printf(" --> %s", buffer);
}


// informacoes de exito ou falha na execussao de comandos
#define debugprint(vals, ...) infoprint(vals, ##__VA_ARGS__)

// informacoes de dados, variaveis, estados, etc
#define logprint(vals, ...)

#elif defined(INFO_LEVEL_MAX)

// informacoes do estado atual e processo de funcionamento do server
void infoprint(const char* vals, ...) {

	char buffer[256];

	va_list va;
	va_start(va, vals);

	vsprintf_s(buffer, vals, va);

	printf(" --> %s", buffer);
}

// informacoes de exito ou falha na execussao de comandos
#define debugprint(vals, ...) infoprint(vals, ##__VA_ARGS__)

// informacoes de dados, variaveis, estados, etc
#define logprint(vals, ...) infoprint(vals, ##__VA_ARGS__)

#endif // INFO_LEVEL

#endif // DEBUG_H

