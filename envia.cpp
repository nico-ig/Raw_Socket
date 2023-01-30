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
// include local
#include "conexao.h"
#include "crc8.cpp"
#include "crc8.h"
#include "frame.h"
#include "macros.h"

using namespace std;

#define HEX "%02x"

int main(int argc, char *argv[]) {
  gen_crc8_table();
  conexao server("lo");
  string dataReceive;
  int receive = 0;
  string dataSend = "Hel";
  cout << "server" << server.get_socket() << endl;
  // while (true) {
  // server.send_data(dataSend, dataSend.size(), 0x01);
  // std::string const HOME = std::getenv("HOME") ? std::getenv("HOME") : ".";
  // std::string const FILENAME = "envia.cpp";
  server.send_file("./foto.jpg");
  cout << "Enviado" << endl;
  // receive = server.receive_data(dataReceive, 1024);
  // cout << "receive: " << receive << endl;
  // }
  return 0;
};