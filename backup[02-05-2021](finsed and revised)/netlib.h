#ifndef NETLIB_H
#define NETLIB_H

/*
Cabecalho para inclusao de funcoes para manipular
o envio de dados em redes para compatbilidade universal
entre dispositivos.

estrutura de inclusao de arquivos:
	
			debug > netdata > netsocket > netlink > netstream > netinit > netlib
	
	debug - para funcoes de exibicao de erros, logs e informacoes revelantes

	netdata - para a estrutura de armazenamento de dados e informacoes sobre os mesmos

	netsocket - para a estrutura de armazenamento de sockets e informacoes da conexao

	netlink - para a estrutura de conexao

	netstream - metodos de Netlink para fluxo de dados

	netinit - metodos de Netlink para conexao, construtores e destrutores, e otros metodos privados

	netlib - arquivo de inclusao para projetos, criando nivel de compatibilidade geral para a biblioteca netlink

    CRIAR SISTEMA DE ERROS QUE NAO QUEBRE O PROGRAMA E SIM RETORNE-OS PARA O PROGRAMA DECIDIR OQUE FAZER
    CRIAR SISTEMA PARA IDENTIFICAR FALNAS NA CONEXAO, POIS É ERRO FACIL DE OCORRER E FACILMENTE TRATADO
*/

// define o nivel de compatbilidade
#if defined(_WIN32) || defined(_WINDOWS_)

// para usar as funcoes padrao de io
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define compatibility_level_data WSADATA wsadata

// desconecta o socket
#define compatibility_level_closesocket(socket) closesocket(socket)

// encerra o uso da .dll ws2_32.lib
#define compatibility_level_cleanup() WSACleanup()

// retorna o ultimo erro ocorrido
#define compatibility_level_geterror() WSAGetLastError()

// inicia a biblioteca winsock
#define compatibility_level_startup() if (verify(WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)) shutdown()

// .dll necessaria para o uso de winsock2.h
#pragma comment(lib, "ws2_32.lib")

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

#define compatibility_level_data

// desconecta o socket
#define compatibility_level_closesocket(socket) close(socket)

// encerra o uso da .dll ws2_32.lib
#define compatibility_level_cleanup()

// retorna o ultimo erro ocorrido
#define compatibility_level_geterror() strerror(errno)

// inicia a biblioteca winsock
#define compatibility_level_startup()

// inclui o vsprintf correspondente
#define compatibility_level_vsprintf(buffer, vals, va) vsprintf(buffer, vals, va)

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
