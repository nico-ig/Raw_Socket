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
  int soquete;
  int device;
  int local, target; // local and target ip address
  vector<frame> framesSending, framesReceiving;
  frame f1, f2;               // frame envio e recebimento
  char buffer[sizeof(frame)]; // buffer
  char bufferSend[sizeof(frame)];
  int sequence = -1;           // sequencia do último frame recebido
  struct sockaddr_ll endereco; // endereco do socket
  vector<int> timeoutValues = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
  
  // ----- Funçoes ------ //
  int ConexaoRawSocket(char *device);
  void receive_frame(char *buffer, int size); // recebe um pacote
  void close_connection();                    // fecha a conexao
                                              
public:
  // ------ Construtor ------ //
  conexao(char *deviceIP);

  /* --------------------- PUBLIC --------------------- */
  /**
   * @brief Construct a new conexao::conexao object
   *
   * @param deviceIP -- ip address of the device
   */
  conexao::conexao(char *deviceIP) { device = ConexaoRawSocket(deviceIP); }

  /**
   * @brief function that sends a frame to the target (dont wait for ack)
   * 
   * @param f 
   * @return int 
   */
  int send_frame(frame *f) {
    int byteSend;
    char buffer[sizeof(frame)];
    memcpy(buffer, f, sizeof(frame));

    bool ack = false;
    int timeout = 0;

    cout << "--------------------------------------------\n";
    cout << "Enviando frame: " << buffer << "\n";
    f->imprime(DEC);
    byteSend = send(device, buffer, sizeof(frame), 0);
    if (byteSend < 0) {
      cout << "Erro no sendto" << byteSend << "\n";
    }
    return byteSend;
  };

  // ------ Funcoes ------ //
  // int get_socket();                               // retorna o socket
  // int send_data(char *buffer, int size, UC tipo); // envia dados
  // int receive_data(char *buffer, int size);       // recebe dados
  // void close_conexao();                           // fecha a conexao

  int get_socket() { return device; };

  int reconstroi_arquivo(string nomeArquivo, vector<frame *> framesFile) {
    cout << "------------Reconstroi arquivo------------------\n";
    cout << "Tamanho do vetor: " << framesFile.size() << "\n";
    string fileData;

    for (size_t i = 0; i < framesFile.size() - 1; i++) {
      fileData += string(framesFile[i]->get_dado(), 63);
    }

    cout << "Tamanho do arquivo: " << fileData.size() << "\n";
    ofstream file;
    file.open("./" + nomeArquivo, ios::binary);
    file << fileData;
    file.close();

    framesFile.clear();

    return 0;
  };

  /**
   * @brief Recebe um frame
   *
   * @return frame*
   */
  frame *receive_frame() {
    char buffer[sizeof(frame)];
    memset(buffer, 0, sizeof(buffer));

    int byteRecv;
    byteRecv = recv(device, buffer, sizeof(frame), 0);
    if (byteRecv <= 0) {
      printf("Erro no recvfrom %d\n", byteRecv);
      return NULL;
    }

    frame *f = new frame;
    memcpy(f, buffer, sizeof(frame));

    cout << "--------------------------------------------\n";
    cout << "Recebido Frame: " << buffer << "\n";

    cout << "Frame:--------------------------------------------\n";
    cout << "binário: ";
    f->imprime(DEC);

    if (f->get_tipo() == ACK) {
      cout << "Recebido um ACK: " << f->get_dado() << "\n";
      return f;
    }

    return f;
  };

  /**
   * @brief Split the selected file into chunks of 63 bytes and send them
   * through the socket with the send_frames function
   *
   * @param location: file location
   * @return int
   */
  vector<frame> send_file(string location) {
    cout << "Location: " << location.c_str() << "\n";
    ifstream file;
    file.open(location.c_str(), ios::binary);

    if (!file) {
      cout << "Erro ao abrir o arquivo1\n";
      return vector<frame>();
    }

    /*-- put file into buffer*/
    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    char *bufferFile = new char[file_size];
    file.read(bufferFile, file_size);
    file.close();

    /*-- add escape chars to the message*/
    string fileData = string(bufferFile, bufferFile + file_size);
    string fileDataEscaped = add_escapes(fileData);

    // --- split file into chunks of 63 bytes --- //
    vector<vector<char>> fileBuffer;
    int byteRead = 0;
    while (byteRead < fileDataEscaped.size()) {
      char chunk[63];
      memset(chunk, 0, sizeof(chunk));
      char buffer[63];
      strncpy(buffer, fileDataEscaped.substr(byteRead, 63).c_str(), 63);
      memcpy(chunk, buffer, 63);
      fileBuffer.push_back(vector<char>(chunk, chunk + sizeof(chunk)));
      byteRead += 63;
    }
    delete[] bufferFile;
    
    // --- create frames from file chunks --- //
    for (size_t i = 0; i < fileBuffer.size(); i++) {
      f1.set_tipo(MIDIA);
      f1.set_seq(i);
      f1.set_dado(string(fileBuffer[i].begin(), fileBuffer[i].end()));
      framesSending.push_back(f1);
    }

    // --- send frames --- //
    return framesSending;
  };
 
  /**
   * @brief verify if the received frame is an ACK and if it is the same as the sent frame
   * 
   * @param received 
   * @param sent 
   * @return true 
   * @return false 
   */
  bool verify_ack(frame *received, frame *sent) {
    if (received->get_tipo() == ACK) {
      if (received->get_seq() == sent->get_seq()) {
        if(received->get_dado() == sent->get_dado()){
          if (received->chk_crc8())
            return true;
        }
      } 
    }
    return false;
  }

  /*-- Não são os escapes certos, só queria deixar pronto essa parte para
     botar os escapes certos--*/
  /**
   * @brief Add escape characters to the data to be sent
   *
   * @param data
   * @return string
   */
  string add_escapes(string data) {
    string message = "";
    for (size_t i = 0; i < data.size(); i++) {
      if (data[i] == 0x7E) {
        message += 0x7D;
        message += 0x5E;
      } else if (data[i] == 0x7D) {
        message += 0x7D;
        message += 0x5D;
      } else {
        message += data[i];
      }
    }
    return message;
  }

  vector<frame*> create_frames(string data) {
    vector<frame*> frames;
    int i = 0;
    string message = add_escapes(data);
    while (i < message.size()) {
      frame *f = new frame();
      f->set_tipo(MIDIA);
      f->set_seq(i);
      f->set_dado(message.substr(i, 63));
      frames.push_back(f);
      i += 63;
    }
    return frames;
  }

 

  int receive_data(string buffer, int size) {
    int byteRecv;
    frame f_recebido;
    char *buffer2 = new char[size];
    byteRecv = recv(soquete, buffer2, size, 0);
    if (byteRecv < 0) {
      printf("Erro no recvfrom %d\n", byteRecv);
      exit(-1);
    }
    // cout << "--------------------------------------------\n";
    // cout << "Recebendo frame: " << buffer2 << "\n"
    //  << "Size: " << byteRecv << "--" << sizeof(frame) << "\n";
    memcpy(&f_recebido, buffer2, sizeof(frame));
    framesReceiving.push_back(f_recebido);
    // cout << "Frame: -----------------------------------------\n";
    // f_recebido.imprime(DEC);
    // cout << "--------------------------------------------\n";
    // cout << "frame.dado: " << f_recebido.get_dado() << "\n";

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
};


/*-- Funções antigas que podem ser usadas --**/

  /*
    int receive_frames(int index) {
      char buffer[sizeof(frame)];
      memset(buffer, 0, sizeof(buffer));

      int byteRecv;
      byteRecv = recv(soquete, buffer, sizeof(frame), 0);
      if (byteRecv <= 0) {
        printf("Erro no recvfrom %d\n", byteRecv);
        exit(-1);
      }
      frame f;
      memcpy(&f, buffer, sizeof(frame));
      if (index <= 5) {
        cout << "--------------------------------------------\n";
        cout << "Recebido Frame: " << index << "\n" << buffer << "\n";
        cout << "Frame:--------------------------------------------\n";
        f.imprime(DEC);
        cout << "framesReceiving.size(): " << framesReceiving.size() << "\n";
      }

      if (f.get_seq() == sequence)
        return f.get_tipo();

      sequence = f.get_seq();
      framesReceiving.push_back(f);
      return f.get_tipo();
    };*/

  // int send_frames(vector<frame> frames) {
  //   int byteSend;
  //   char buffer[sizeof(frame)];
  //   cout << "Tamanho do vetor: " << frames.size() << "\n";

  //   // Send INI frame
  //   frame f;
  //   f.set_tipo(INIT);
  //   f.set_seq(0);
  //   f.set_dado("INI");
  //   memcpy(buffer, &f, sizeof(frame));
  //   send_frame(f);

  //   cout << "--------------------------------------------\n";
  //   cout << "Enviando INI: " << buffer << "\n";
  //   cout << "Frame: -----------------------------------------\n";
  //   f.imprime(DEC);

  //   int i = 0;
  //   for (i = 0; i < frames.size(); i++) {
  //     send_frame(frames[i]);
  //     // memset(buffer, 0, sizeof(buffer));
  //     // memcpy(buffer, &frames[i], sizeof(frame));

  //     // frame f;
  //     // memcpy(&f, buffer, sizeof(frame));
  //     // byteSend = send(soquete, buffer, sizeof(frame), 0);
  //     // if (byteSend < 0) {
  //     //   printf("Erro no sendto %d\n", byteSend);
  //     // }
  //     // if (i <= 5) {
  //     //   cout << "--------------------------------------------\n";
  //     //   cout << "Enviando frame: " << i << "\n" << buffer << "\n";
  //     //   cout << "Frame: -----------------------------------------\n";
  //     //   f.imprime(DEC);
  //     // }
  //   }

  //   cout << "--------------------------------------------\n";
  //   cout << "Enviado " << i << " frames\n";
  //   // send end of file
  //   frame fim;
  //   fim.set_tipo(FIMT);
  //   fim.set_seq(0);
  //   fim.set_dado("FIM");
  //   byteSend = send_frame(fim);
  //   // memcpy(buffer, &f, sizeof(frame));
  //   // cout << "--------------------------------------------\n";
  //   // cout << "Enviando FIM: " << buffer << "\n";
  //   // cout << "Frame: -----------------------------------------\n";
  //   // fim.imprime(DEC);

  //   // byteSend = send(soquete, buffer, sizeof(frame), 0);
  //   // if (byteSend < 0) {
  //   //   printf("Erro no sendto FIMT %d\n", byteSend);
  //   //   exit(-1);
  //   // }

  //   return byteSend;
  // }

#endif
