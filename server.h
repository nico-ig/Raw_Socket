// Receive all the data
#ifndef _SERVER_
#define _SERVER_

#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
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

#include <arpa/inet.h>

// include local
#include "conexao.h"
#include "crc8.h"
#include "frame.h"
#include <mutex>
std::mutex mtx;

using namespace std;

class server {

private:
  int soquete;
  vector<frame *> framesMidia;
  conexao *local;
  conexao *target;
  int send_nack(frame fReceive);
  int send_ack(frame fReceive);

public:
  server(conexao *local, conexao *target);
  // ~server();
  void run();
};

/**
 * @brief function that sends a ack frame to the target
 *
 * @param fReceive frame received
 * @return int
 */
int server::send_ack(frame fReceive) {
  frame *ack = new frame();
  ack->set_tipo(ACK);
  ack->set_dado(fReceive.get_dado());
  ack->set_seq(fReceive.get_seq());
  cout << "Enviando ACK ---------------\n";
  int ackSent = 0;
  ackSent = target->send_frame(ack);
  cout << "ACK enviado: " << ackSent << endl;
  return ackSent;
};

/**
 * @brief function that sends a nack frame to the target
 *
 * @param fReceive frame received
 * @return int
 */
int server::send_nack(frame fReceive) {
  frame *nack = new frame();
  nack->set_tipo(NACK);
  nack->set_dado(fReceive.get_dado());
  nack->set_seq(fReceive.get_seq());
  cout << "Enviando NACK ---------------\n";
  int nackSent = 0;
  nackSent = target->send_frame(nack);
  cout << "NACK enviado: " << nackSent << endl;
  return nackSent;
};

/**
 * @brief function that runs the server thread and receives the frames
 *
 */
void server::run() {
  // lock the thread with mutex
  int i;
  while (true) {
    frame fReceive;
    cout << "Recebendo frame ---------------\n";

    /*-- listening local ip and wainting for messages --*/
    fReceive = *local->receive_frame();
    fReceive.imprime(DEC);

    /*-- if frame is an ACK or NACK, it ignores it --*/
    bool sendAck = fReceive.chk_crc8() && fReceive.get_tipo() != ACK &&
                   fReceive.get_tipo() != NACK;
    bool sendNack = !fReceive.chk_crc8() && fReceive.get_tipo() != ACK &&
                    fReceive.get_tipo() != NACK;
    /*-- checking crc and sending ack or nack --*/
    if (sendAck)
      send_ack(fReceive);
    else if (sendNack)
      send_nack(fReceive);
  }
};

/**
 * @brief Construct a new server::server object
 *
 * @param localConnection local ip and port to listen
 * @param targetConnection target ip and port to send
 */
server::server(conexao *localConnection, conexao *targetConnection) {
  local = localConnection;
  target = targetConnection;
}

#endif
