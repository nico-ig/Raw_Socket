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
#include <thread>

// include local
#include "conexao.h"
#include "crc8.cpp"
#include "crc8.h"
#include "frame.h"
#include "macros.h"
#include "server.h"
#include "client.h"

using namespace std;

#define HEX "%02x"

int main(int argc, char *argv[]) {
  gen_crc8_table();

  conexao local("lo");
  conexao target("lo1");

  server servidor(&local, &target);
  thread serverReceive(&server::run, &servidor);

  client cliente(&local, &target);
  thread clientSend(&client::run, &cliente);
  string dataSend;
  string dataReceive;
  
  int receive = 0;
  while (true) {
    receive++;
  }

  serverReceive.join();
  clientSend.join();

  return 0;
}

