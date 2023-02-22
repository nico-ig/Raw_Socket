#ifndef _CONEXAO_
#define _CONEXAO_

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
#include "crc8.h"
#include "frame.h"

using namespace std;

#define BYTE "%02x"

class conexao {
private:
  // ------ Dados ------ //
  int sequence = -1;                  // sequencia do último frame recebido
  int local, target;                  // local and target ip address
                     
  frame f1, f2;                       // frame envio e recebimento
  vector<frame> framesSending;
  vector<frame> framesReceiving;

  char bufferReceived[sizeof(frame)]; // buffer
  char bufferSend[sizeof(frame)];

                               
  int device;
  int soquete;
  struct sockaddr_ll endereco;        // endereco do socket
                               
  vector<int> timeoutValues = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
  
  // ----- Funçoes ------ //
  int  ConexaoRawSocket(char *device);
  void close_connection();             // fecha a conexao
                                              
public:
  // ------ Construtor ------ //
  conexao(char *deviceIP);
  frame *receive_frame();              // recebe um pacote
  int send_frame(frame *f);            // envia um pacote

  int get_socket() { return device; };
};

// ------------------------------- PUBLIC --------------------------------- //

/**
 * @brief Construct a new conexao::conexao object
 *
 * @param deviceIP -- ip address of the device
 */
conexao::conexao(char *deviceIP) { device = ConexaoRawSocket(deviceIP); }

/**
* @brief Recebe um frame
*
* @return frame*
*/
frame *conexao::receive_frame() {

  memset(bufferReceived, 0, sizeof(bufferReceived));

  int byteRecv = recv(device, bufferReceived, sizeof(frame), 0);
  if (byteRecv <= 0) {
    printf("Erro no recvfrom %d\n", byteRecv);
    return NULL;
  }

  frame *f = new frame;
  memcpy(f, bufferReceived, sizeof(frame));

  cout << "--------------------------------------------\n";
  cout << "Recebido Frame: " << bufferReceived << "\n";

  cout << "Frame:--------------------------------------------\n";
  cout << "binário: ";
  f->imprime(DEC);

  if (f->get_tipo() == ACK) 
    cout << "Recebido um ACK: " << f->get_dado() << "\n";

  return f;
}

/**
* @brief function that sends a frame to the target (dont wait for ack)
* 
* @param f 
* @return int 
*/
int conexao::send_frame(frame *f) {
  int byteSend;
  memcpy(bufferSend, f, sizeof(frame));

  bool ack = false;
  int timeout = 0;

  cout << "--------------------------------------------\n";
  cout << "Enviando frame: " << bufferSend << "\n";
  f->imprime(DEC);
  
  byteSend = send(device, bufferSend, sizeof(frame), 0);
  if (byteSend < 0) {
    cout << "Erro no sendto" << byteSend << "\n";
  }

  return byteSend;
}

// ------------------------------ PRIVATE --------------------------------- //

int conexao::ConexaoRawSocket(char *device) {
  int soquete;
  struct ifreq ir;
  struct sockaddr_ll endereco;
  struct packet_mreq mr;

  soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); /*cria socket*/
  if (soquete == -1) {
    printf("Erro no Socket\n");
    exit(-1);
  }

  memset(&ir, 0, sizeof(struct ifreq)); /*dispositivo eth0*/
  memcpy(ir.ifr_name, device, sizeof(device));
  if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
    printf("Erro no ioctl\n");
    exit(-1);
  }

  memset(&endereco, 0, sizeof(endereco)); /*IP do dispositivo*/
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ir.ifr_ifindex;
  if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
    printf("Erro no bind\n");
    exit(-1);
  }

  memset(&mr, 0, sizeof(mr)); /*Modo Promiscuo*/
  mr.mr_ifindex = ir.ifr_ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  if ((setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) <
       0)) {
    printf("Erro ao fazer setsockopt\n");
    exit(-1);
  }
  if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
    perror("ioctl error");
    return -1;
  }

  return soquete;
}

#endif
