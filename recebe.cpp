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
// include local
#include "conexao.h"
#include "crc8.cpp"
#include "crc8.h"
#include "frame.h"
#include "macros.h"
#include "server.h"
using namespace std;

#define HEX "%02x"

int main(int argc, char *argv[]) {
  gen_crc8_table();

  conexao connection("lo");
  server servidor("lo1");
  thread serverReceive(&server::run, &servidor);
  serverReceive.join();
  string dataReceive;
  int receive = 0;
  // cout << "server" << connection.get_socket() << endl;
  // int receiveds = 0;
  // vector<frame *> frames;
  
  // while (true) {

  //   frame *f = new frame();
  //   f = connection.receive_frame(true);
  //   if (f->get_tipo() == MIDIA)
  //     frames.push_back(f);

  //   receiveds++;
  //   // cout << "receive: " << receive << endl;
  //   if (f->get_tipo() == FIMT) {
  //     cout << "------------------ FIMT ------------------\n";
  //     connection.reconstroi_arquivo("recebe_Arquivo", frames);
  //     break;
  //   }
  //   // reconstroi arquivo
  // }

  return 0;
}

/*
 // send ack
    if (!sendAck)
      return f;

    cout << "Enviando ACK ---------------\n";
    frame ack;
    ack.set_tipo(ACK);
    ack.set_seq(f->get_seq());
    ack.set_dado(f->get_dado());
    send_frame(ack, false);
    cout << "Frame ACK:--------------------------------------------\n";
    ack.imprime(DEC);
    int byteSend = send(soquete, bufferSend, sizeof(frame), 0);
    sleep(1);
    if (byteSend < 0) {
      cout << "Erro ao mandar o ACK" << byteSend << "\n";
    }



*/