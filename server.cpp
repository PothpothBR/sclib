#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define INFO_LEVEL_MAX

#include <iostream>
#include "lastest/netlib.h"

using namespace std;

int main(){

    NETDATA data("seu buceta!");

    Netlink server_fofinho(SERVER_SIDE, 0, "27094");

    server_fofinho.waitconnection();

    server_fofinho.lastsocketid() = 22;

    cout << "enviando data\n";

    server_fofinho.senddata(&data, 22);

    server_fofinho.recvdata(&data, 22);

    cout << (char*)data.data << endl;

    cin.get();
    return 0;
}
