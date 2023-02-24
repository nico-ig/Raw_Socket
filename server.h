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
#include <mutex>

#include <arpa/inet.h>

// include local
#include "conexao.h"
#include "frame.h"

using namespace std;

class server {

private:
  // --------- Dados ---------- //
  int soquete;

  conexao *socket;

  vector<frame *> framesMidia;

  // ---------- Funcoes -------- //
  int send_nack(frame *fReceive);
  int send_ack(frame *fReceive);

public:
  // ------- Construtores ------ //
  server(conexao *socketAddr) { socket = socketAddr; };

  // ---------- Funcoes -------- //
  void run();
};

// ------------------------------ PRIVATE --------------------------------- //
/**
 * @brief function that sends a ack frame to the target
 *
 * @param fReceive frame received
 * @return int
 */
int server::send_ack(frame *fReceive) {

  frame *ack = new frame();

  ack->set_tipo(ACK);
  ack->set_dado(vector<char>(fReceive->get_dado(), fReceive->get_dado()+fReceive->get_tam()));
  ack->set_seq(fReceive->get_seq());

  int ackSent = 0;
  ackSent = socket->send_frame(ack);

  cout << "ACK enviado\n";

  return ackSent;
}

/**
 * @brief function that sends a nack frame to the target
 *
 * @param fReceive frame received
 * @return int
 */
int server::send_nack(frame *fReceive) {

  frame *nack = new frame();

  nack->set_tipo(NACK);
  nack->set_dado(vector<char>(fReceive->get_dado(), fReceive->get_dado()+fReceive->get_tam()));
  nack->set_seq(fReceive->get_seq());

  int nackSent = 0;
  nackSent = socket->send_frame(nack);

  cout << "NACK enviado\n";

  return nackSent;
}


// ------------------------------- PUBLIC --------------------------------- //

/**
 * @brief function that runs the server thread and receives the frames
 *
 */

// lock the thread with mutex
void server::run() {

  while (true) {
    frame *fReceive;

    /*-- listening local ip and waiting for messages --*/
    /*-- ignore if the package is not valid          --*/
    if ( ! (fReceive = socket->receive_frame()) ) { continue; }
    cout << "Frame recebido:" << endl;
    fReceive->imprime(HEX);

    /*-- if frame is an ACK or NACK, it ignores it --*/
    bool sendAck = fReceive->chk_crc8() && (fReceive->get_tipo() != ACK) &&
                   (fReceive->get_tipo() != NACK);
    bool sendNack = !fReceive->chk_crc8() && fReceive->get_tipo() != ACK &&
                    fReceive->get_tipo() != NACK;

    /*-- checking crc and sending ack or nack --*/
    if (sendAck)
      send_ack(fReceive);
    else if (sendNack)
      send_nack(fReceive);
  }
}

#endif
