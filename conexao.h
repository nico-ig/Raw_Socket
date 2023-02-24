#ifndef _CONEXAO_
#define _CONEXAO_

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

// include local
#include "frame.h"

using namespace std;

#define BYTE "%02x"

class conexao {
private:
  // ------ Dados ------ //
  int timeoutMillis = 500;            // Tempo que fica tentando ler
  int sequence = -1;                  // sequencia do último frame recebido
  int local, target;                  // local and target ip address
                     
  frame f1, f2;                       // frame envio e recebimento
  vector<frame> framesSending;
  vector<frame> framesReceiving;

  char bufferReceived[sizeof(frame) * 2]; // buffer
  char bufferSend[sizeof(frame) * 2];
                               
  int soquete;
                               
  vector<int> timeoutValues = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
  
  // ----- Funçoes ------ //
  int add_escapes(char *f, char* out);
  int remove_escapes(char *f, char *out);
  int  ConexaoRawSocket(char *nome_interface_rede);
  long long timestamp();
  void close_connection();             // fecha a conexao
                                              
public:
  // ------ Construtor ------ //
  conexao(char *deviceIP);
  frame *receive_frame();              // recebe um pacote
  int send_frame(frame *f);            // envia um pacote
};

// ------------------------------- PUBLIC --------------------------------- //

/**
 * @brief Construct a new conexao::conexao object
 *
 * @param deviceIP -- ip address of the device
 */
conexao::conexao(char *deviceIP) { soquete = ConexaoRawSocket(deviceIP); memset(bufferSend, 0, sizeof(frame)*2); }

/**
* @brief Recebe um frame
*
* @return frame*
*/
frame *conexao::receive_frame() {

  int byteRecv;
  int lastSeq = -1;
  long long start = timestamp();
  struct timeval timeout = { .tv_sec = 0, .tv_usec = timeoutMillis * 1000 };
  setsockopt(soquete, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

  do {
    byteRecv = recv(soquete, bufferReceived, sizeof(frame) * 2, 0);
//    for (int i = 0; i < byteRecv; i++) {
//      cout << hex << (int(bufferReceived[i])&0xff) << " ";
//    }
    cout << "\n";
    if ( (byteRecv > 0) && (bufferReceived[0] == INI) ) {
      frame *f = new frame;
      remove_escapes(bufferReceived, (char*)f);
      return f;
    }
  } while ( timestamp() - start <= timeoutMillis );

  return NULL;
}

/**
* @brief function that sends a frame to the target (dont wait for ack)
* 
* @param f 
* @return int 
*/
int conexao::send_frame(frame *f) {

  add_escapes((char*) f, bufferSend);

  bool ack = false;
  int timeout = 0;

  int byteSend = send(soquete, bufferSend, sizeof(frame) * 2, 0);
  printf("send %d: ", byteSend);
  for (int i = 0; i < byteSend; i++) {
    cout << hex << (int(bufferSend[i])&0xff) << " ";
  }
  cout << "\n";
  if (byteSend < 0) {
    cout << "Erro no sendto" << byteSend << "\n";
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

    if ( f[i] == 0x88 || f[i] == 0x81 )
      out[j++] = 0xFF;
  }

  return j;
}

int conexao::remove_escapes(char *f, char* out) {
  int j = 0;
  for (size_t i = 0; j < sizeof(frame); i++)
  {
    out[j++] = f[i];

    if ( f[i] == 0x88 || f[i] == 0x81 )
      i++;
  }

  return j;
}

int conexao::ConexaoRawSocket(char *nome_interface_rede) {
  // Cria o socket sem nenhum protocolo
  int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); /*cria socket*/
  if (soquete == -1) {
    printf("Erro no Socket, verifique se voce eh root\n");
    exit(-1);
  }

  int ifindex = if_nametoindex(nome_interface_rede);

  struct sockaddr_ll endereco = {0};
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ifindex;

  if ( bind(soquete, (struct sockaddr*)&endereco, sizeof(endereco)) == -1 )
  {
    printf("Erro no bind\n");
    exit(-1);
  }

  struct packet_mreq mr = {0};
  mr.mr_ifindex = ifindex;
  mr.mr_type = PACKET_MR_PROMISC;

//  memset(&ir, 0, sizeof(struct ifreq)); /*dispositivo eth0*/
//  memcpy(ir.ifr_name, device, sizeof(device));
//  if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
//    printf("Erro no ioctl\n");
//    exit(-1);
//  }
//
//  memset(&endereco, 0, sizeof(endereco)); /*IP do dispositivo*/
//  endereco.sll_family = AF_PACKET;
//  endereco.sll_protocol = htons(ETH_P_ALL);
//  endereco.sll_ifindex = ir.ifr_ifindex;
//  if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
//    printf("Erro no bind\n");
//    exit(-1);
//  }
//
//  memset(&mr, 0, sizeof(mr)); /*Modo Promiscuo*/
//  mr.mr_ifindex = ir.ifr_ifindex;
//  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
    printf("Erro ao fazer setsockopt\n");
    printf("Verifique a especificacao da placa: eth0, enp3s0, ...\n");
    exit(-1);
  }
  
  return soquete;
}

long long conexao::timestamp()
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

#endif
