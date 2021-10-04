#ifndef NETLIB_H
#define NETLIB_H

/*
Cabecalho para inclusao de funcoes para manipular
o envio de dados em redes para compatbilidade universal
entre dispositivos.

Sendo este um nivel de compatbilidade para windows.

estrutura de inclusao de arquivos:

							 
			debug > netdata > netlink > netstream > netinit > netlib

							
	debug - para funcoes de exibicao de erros, logs e informacoes revelantes

	netdata - para a estrutura de armazenamento de dados

	netlink - para a estrutura de conexao

	netstream - metodos de Netlink para fluxo de dados

	netinit - metodos de Netlink para conexao, construtores e destrutores, e otros metodos privados


	netlib - arquivo de inclusao para projetos, criando nivel de compatibilidade geral para a biblioteca netlink

*/

// define o nivel de compatbilidade
#if defined(_WIN32) || defined(_WINDOWS_) || defined(WINDOWS_GNU_COMPATIBILITY_LEVEL)

#define compatbility_level_data WSADATA wsadata

// desconecta o socket
#define compatbility_level_closesocket(socket) closesocket(socket)

// encerra o uso da .dll ws2_32.lib
#define compatbility_level_cleanup() WSACleanup()

// retorna o ultimo erro ocorrido
#define compatbility_level_geterror() WSAGetLastError()

// inicia a biblioteca winsock
#define compatbility_level_startup() if (verify(WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)) shutdown()

// define o uso de vsprintf com compiladores gnu gcc - mingw
#if defined(__GNUG__) || defined(__GNUC__) || defined(WINDOWS_GNU_COMPATIBILITY_LEVEL)
#define compatbility_level_vsprintf(buffer, vals, va) vsprintf(buffer, vals, va)

// define o uso de vsprintf com compiladores vs++ e o linker automatico da ws2_32 dll
#else
#define compatbility_level_vsprintf(buffer, vals, va) vsprintf_s(buffer, vals, va)

// .dll necessaria para o uso de winsock2.h
#pragma comment(lib, "ws2_32.lib")
#endif // __GNUG__ || __GNUC__

// para o uso de sockets
#include <winsock2.h>
#include <WS2tcpip.h>

// exit()
#include <stdlib.h>

// strerror() - memset()
#include <string.h>

// clock_t - clock()
#include <time.h>

// strlen() - vsprintf
#include <stdio.h>

//inclui netlink e todos os seus metodos
#include "netlink/netinit.h"

#elif defined(LINUX) || defined(_LINUX_)

#define compatbility_level_data

// desconecta o socket
#define compatbility_level_closesocket(socket) close(socket)

// encerra o uso da .dll ws2_32.lib
#define compatbility_level_cleanup()

// retorna o ultimo erro ocorrido
#define compatbility_level_geterror() strerror(errno)

// inicia a biblioteca winsock
#define compatbility_level_startup()

// inclui o vsprintf correspondente
#define compatbility_level_vsprintf(buffer, vals, va) vsprintf(buffer, vals, va)

// errno
#include <errno.h>

// close()
#include <unistd.h>

// IPPROTO_TCP - INADDR_ANY
#include <arpa/inet.h>
#include <netinet/in.h>

// socket() - bind() - listen() - accept() - connect() - send() - recv()
#include <socket.h>

//inclui netlink e todos os seus metodos
#include "netlink/netinit.h"

// addrinfo
#include <netdb.h>

#endif // LINUX

#endif // NETLIB_H
