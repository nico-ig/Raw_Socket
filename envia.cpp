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
  conexao local("lo1");
  conexao target("lo");
  string dataReceive;
  int receive = 0;
  string dataSend = "Hel";
  cout << "envia" << target.get_socket() << endl;
  // while (true) {
  // server.send_data(dataSend, dataSend.size(), 0x01);
  // std::string const HOME = std::getenv("HOME") ? std::getenv("HOME") : ".";
  // std::string const FILENAME = "envia.cpp";
  // vector<frame> fileFrames = envia.send_file("./foto.jpg");

  // send file frames
  //  for (int i = 0; i < fileFrames.size(); i++) {
  //    fileFrames[i].imprime(DEC);
  //    int frameSend = envia.send_frame(&fileFrames[i]);
  //    cout << "frameSend: " << frameSend << endl;
  //    frame *f = recebe.receive_frame(false);
  //    if (f->get_tipo() == ACK) {
  //      cout << "ACK" << endl;
  //    }
  //  }

  // send hello world
  frame *f = new frame();
  f->set_tipo(TEXTO);
  f->set_dado("Hello World");
  f->set_seq(0);
  f->add_crc(f->get_dado());
  int frameSend = target.send_frame(f);

  bool ack = false;
  int timeSend = time(NULL);
  int timeout = 0;
  int timeouts[] = {2, 4, 8, 16, 32};
  while (!ack && timeout < 5 && time(NULL)) {
    frame *f2 = local.receive_frame(false);
    if (f2->get_tipo() == ACK) {
      cout << "------------------ ACK ------------------\n" << endl;
      f2->imprime(DEC);
      cout << "ACK" << endl;
      ack = true;
    } else
      sleep(timeouts[timeout]);
  }

  // frame *f2 = recebe.receive_frame(false);
  // if (f2->get_tipo() == ACK) {
  //   cout << "ACK" << endl;
  // }
  // // cout << "server" << connection.get_socket() << endl;
 
  cout << "Enviado" << endl;
  // receive = server.receive_data(dataReceive, 1024);
  // cout << "receive: " << receive << endl;
  // }
  return 0;
};