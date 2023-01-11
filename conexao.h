#ifndef _CONEXAO_
#define _CONEXAO_

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
#include "frame.h"

using namespace std;

#define BYTE "%02x"

class conexao {
private:
  // ------ Dados ------ //

  int soquete;
  vector<frame> frames;
  frame f1, f2; // frame envio e recebimento
  char buffer[2024];
  char bufferSend[2024] = "Hello World ";
  struct sockaddr_ll endereco;

  // ----- Funçoes ------ //

  int ConexaoRawSocket(char *device);
  void send_frame(char *buffer, int size);    // envia um pacote
  void receive_frame(char *buffer, int size); // recebe um pacote
  void close_connection();                    // fecha a conexao
public:
  // ------ Construtor ------ //
  conexao();
  conexao(char *device);

  // ------ Funcoes ------ //
  // int get_socket();                               // retorna o socket
  // int send_data(char *buffer, int size, UC tipo); // envia dados
  // int receive_data(char *buffer, int size);       // recebe dados
  // void close_conexao();                           // fecha a conexao

  int get_socket() { return soquete; }

  int send_data(string data, int size, UC tipo) {
    int byteSend;
    if (data.size() > BITPOW(6) - 1) {
      // Vector of frames
      vector<frame> frames;
      for (size_t i = 0; i < data.size(); i += BITPOW(6) - 1) {
        frame f;
        f.set_tipo(tipo);
        f.set_seq(i);
        f.set_dado(data.substr(i, BITPOW(6) - 1));
        frames.push_back(f);
      }
      for (size_t i = 0; i < frames.size(); i++) {
        frames[i].imprime();
        memcpy(buffer, &frames[i], sizeof(frame));
        byteSend = send(soquete, buffer, sizeof(frame), 0);
        if (byteSend < 0) {
          printf("Erro no sendto %d\n", byteSend);
          exit(-1);
        }
      }
    } else {
      f1.set_tipo(0x10);
      f1.set_seq(0x01);
      f1.set_dado(data);
      char bufferSend[1024] = "Hello World ";
      memcpy(bufferSend, &f1, sizeof(frame));
      cout << "--------------------------------------------\n";
      cout << "Enviando frame: " << bufferSend << "\n";
      cout << "Frame: -----------------------------------------\n";
      f1.imprime();

      byteSend = send(soquete, bufferSend, 1024 - 1, 0);
      if (byteSend < 0) {
        printf("Erro no sendto %d\n", byteSend);
        exit(-1);
      }
    }
    return byteSend;
  };

  int receive_data(string buffer, int size) {
    int byteRecv;
    frame f_recebido;
    char *buffer2 = new char[size];
    byteRecv = recv(soquete, buffer2, size, 0);
    if (byteRecv < 0) {
      printf("Erro no recvfrom %d\n", byteRecv);
      exit(-1);
    }
    cout << "--------------------------------------------\n";
    cout << "Recebendo frame: " << buffer2 << "\n"
         << "Size: " << byteRecv << "--" << sizeof(frame) << "\n";
    memcpy(&f_recebido, buffer2, sizeof(frame));
    cout << "Frame: -----------------------------------------\n";
    f_recebido.imprime();
    cout << "--------------------------------------------\n";
    cout << "frame.dado: " << f_recebido.get_dado() << "\n";

    return byteRecv;
  }
};

/* --------------------- PRIVATE --------------------- */

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
  if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) ==
      -1) {
    printf("Erro ao fazer setsockopt\n");
    exit(-1);
  }

  return soquete;
};

/* --------------------- PUBLIC --------------------- */

conexao::conexao(char *device) { soquete = ConexaoRawSocket(device); }

// conexao::sendFrame(char *buffer, int size) {
//   int byteSend;
//   byteSend = send(soquete, buffer, size, 0);
//   if (byteSend < 0) {
//     printf("Erro no sendto %d\n", byteSend);
//     exit(-1);
//   }
// }g

// int main(int argc, char *argv[]) {
//   int soquete;
//   char buffer[1024];
//   char bufferSend[1024] = "Hello World ";
//   // void *__restrict__ __buffer2;
//   // struct sockaddr_ll endereco;
//   // socklen_t tamanho = sizeof(endereco);

//   soquete = ConexaoRawSocket("lo"); /*cria socket*/

//   int byteRead, byteSend;
//   int counter = 0;
//   while (counter++ < 5) {
//     strcat(bufferSend, to_string(counter).c_str());
//     byteSend = send(soquete, bufferSend, 1024 - 1, 0);
//     byteRead = recv(soquete, buffer, 1024 - 1, 0);

//     // send data to socket

//     if (byteRead < 0) {
//       printf("Erro no recvfrom %d\n", byteRead);
//       exit(-1);
//     }
//     buffer[byteRead] = 0;
//     // c++ print every byte in buffer

//     printf("recebido %d Bytes: -> %s \n", byteRead, buffer);
//     for (int i = 0; i < byteRead; i++) {
//       if ((unsigned int)buffer[i] > 0 && (unsigned int)buffer[i] < 126) {
//         printf("%02x", (unsigned int)buffer[i]);

//         /* code */
//       }
//     }
//     printf("\n");
//     printf("-------------------\n");
//   }
//   return 0;
// };

#endif