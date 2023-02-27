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
#include <sys/time.h>
#include <sys/types.h>

#include <arpa/inet.h>

// include local
#include "cores.h"
#include "frame.h"
using namespace std;

#define NUM_RETRIES 10
#define TAM_JANELA 2

class conexao {
private:
  // ------ Dados ------ //
  
  int sequence = -1;       // sequencia do último frame recebido
  int local, target;       // local and target ip address

  frame f1, f2; // frame envio e recebimento
  vector<frame> framesSending;
  vector<frame> framesReceiving;

  char bufferReceived[sizeof(frame) * 2]; // buffer
  char bufferSend[sizeof(frame) * 2];

  int device;
  struct sockaddr_ll endereco; // endereco do socket

  // ----- Funçoes ------ //
  int add_escapes(char *f, char *out);
  int remove_escapes(char *f, char *out);
  int ConexaoRawSocket(char *device);
  void close_connection(); // fecha a conexao

public:
  int timeoutMillis = 200; // Tempo que fica tentando ler

  // ------ Construtor ------ //
  conexao(char *deviceIP);
  frame *receive_frame();   // recebe um pacote
  int send_frame(frame *f); // envia um pacote
  long long timestamp();

  int get_socket() { return device; };
};

// ------------------------------- PUBLIC --------------------------------- //

/**
 * @brief Construct a new conexao::conexao object
 *
 * @param deviceIP -- ip address of the device
 */
conexao::conexao(char *deviceIP) {
  device = ConexaoRawSocket(deviceIP);
  memset(bufferSend, 0, sizeof(frame) * 2);
}

/**
 * @brief Recebe um frame
 *
 * @return frame*
 */
frame *conexao::receive_frame() {

  int byteRecv;
  int lastSeq = -1;
  long long start = timestamp();
  struct timeval timeout = {.tv_sec = 0, .tv_usec = timeoutMillis * 1000};
  setsockopt(device, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
             sizeof(timeout));

  do {
    byteRecv = recv(device, bufferReceived, sizeof(frame) * 2, 0);
    if ((byteRecv > 0) && (bufferReceived[0] == INI)) {
      frame *f = new frame;
      remove_escapes(bufferReceived, (char *)f);
      return f;
    }
  } while (timestamp() - start <= timeoutMillis);
  return NULL;
}

/**
 * @brief function that sends a frame to the target (dont wait for ack)
 *
 * @param f
 * @return int
 */
int conexao::send_frame(frame *f) {

  add_escapes((char *)f, bufferSend);

  bool ack = false;
  int timeout = 0;

  int byteSend = send(device, bufferSend, sizeof(frame) * 2, 0);
  printf("send %d: ", byteSend);
  for (int i = 0; i < byteSend; i++) {
    cout << hex << (int(bufferSend[i]) & 0xff) << " ";
  }
  cout << "\n";
  if (byteSend < 0) {
    cout << "Erro no sendto" << byteSend << "\n";
    return -1;
  }

  return byteSend;
}

// ------------------------------ PRIVATE --------------------------------- //

/**
 * @brief Add escape characters to the data to be sent
 *
 * @param data
 * @return string
 */
int conexao::add_escapes(char *f, char *out) {

  int j = 0;

  for (size_t i = 0; i < sizeof(frame); i++) {
    out[j++] = f[i];

    if (f[i] == 0x88 || f[i] == 0x81) out[j++] = 0xFF;
  }

  return j;
}

int conexao::remove_escapes(char *f, char *out) {
  int j = 0;
  for (size_t i = 0; j < sizeof(frame); i++) {
    out[j++] = f[i];

    if (f[i] == 0x88 || f[i] == 0x81) i++;
  }

  return j;
}

int conexao::ConexaoRawSocket(char *device) {
  int soquete;
  struct ifreq ir;
  struct sockaddr_ll endereco;
  struct packet_mreq mr;

  soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); /*cria socket*/
  if (soquete == -1) {
    cout << BOLDRED << "\tErro no Socket, verifique se voce eh root\n" << RESET;
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

  return soquete;
}

long long conexao::timestamp() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

#endif
