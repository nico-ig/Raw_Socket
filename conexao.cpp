#include <bits/stdc++.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

// include local
#include "conexao.h"

using namespace std;

#define HEX "%02x"

int main(int argc, char *argv[]) {

  conexao server("lo");
  string dataReceive;
  int receive = 0;
  string dataSend = "Hello World";
  cout << "server" << server.get_socket() << endl;

  while (true) {
    server.send_data(dataSend, 12, 0x01);
    receive = server.receive_data(dataReceive, 1024);
    cout << "receive: " << receive << endl;
  }

  return 0;
}
