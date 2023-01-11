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
using namespace std;

// include local
#include "conexao.h"

#define HEX "%02x"

// int ConexaoRawSocket(char *device) {
//   int soquete;
//   struct ifreq ir;
//   struct sockaddr_ll endereco;
//   struct packet_mreq mr;

//   soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); /*cria socket*/
//   if (soquete == -1) {
//     printf("Erro no Socket\n");
//     exit(-1);
//   }

//   memset(&ir, 0, sizeof(struct ifreq)); /*dispositivo eth0*/
//   memcpy(ir.ifr_name, device, sizeof(device));
//   if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
//     printf("Erro no ioctl\n");
//     exit(-1);
//   }

//   memset(&endereco, 0, sizeof(endereco)); /*IP do dispositivo*/
//   endereco.sll_family = AF_PACKET;
//   endereco.sll_protocol = htons(ETH_P_ALL);
//   endereco.sll_ifindex = ir.ifr_ifindex;
//   if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
//     printf("Erro no bind\n");
//     exit(-1);
//   }

//   memset(&mr, 0, sizeof(mr)); /*Modo Promiscuo*/
//   mr.mr_ifindex = ir.ifr_ifindex;
//   mr.mr_type = PACKET_MR_PROMISC;
//   if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr))
//   ==
//       -1) {
//     printf("Erro ao fazer setsockopt\n");
//     exit(-1);
//   }

//   return soquete;
// }

int main(int argc, char *argv[]) {

  conexao server("lo");
  string dataReceive;
  int receive = 0;
  string dataSend = "Hello World";
  cout << "server" << server.get_socket() << endl;
  while (true) {
    server.send_data(dataSend, 12, 0x01);
    receive = server.receive_data(dataReceive, 1024);
    cout << "receive: " << receive << endl;
  }

  // int soquete;
  // char buffer[1024];
  // char bufferSend[1024] = "Hello World ";
  // // void *__restrict__ __buffer2;
  // // struct sockaddr_ll endereco;
  // // socklen_t tamanho = sizeof(endereco);

  // soquete = ConexaoRawSocket("lo"); /*cria socket*/

  // int byteRead, byteSend;
  // int counter = 0;
  // while (counter++ < 5) {
  //   strcat(bufferSend, to_string(counter).c_str());
  //   byteSend = send(soquete, bufferSend, 1024 - 1, 0);
  //   byteRead = recv(soquete, buffer, 1024 - 1, 0);

  //   // send data to socket

  //   if (byteRead < 0) {
  //     printf("Erro no recvfrom %d\n", byteRead);
  //     exit(-1);
  //   }
  //   buffer[byteRead] = 0;
  //   // c++ print every byte in buffer

  //   printf("recebido %d Bytes: -> %s \n", byteRead, buffer);
  //   for (int i = 0; i < byteRead; i++) {
  //     if ((unsigned int)buffer[i] > 0 && (unsigned int)buffer[i] < 126) {
  //       printf("byte->" HEX, (unsigned int)buffer[i]);

  //       /* code */
  //     }
  //   }
  //   printf("\n");
  //   printf("-------------------\n");
  // }
  return 0;
}