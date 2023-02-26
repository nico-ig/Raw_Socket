// Receive all the data
#ifndef _SERVER_
#define _SERVER_

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <mutex>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h> // For stat() and mkdir() functions
#include <sys/statvfs.h>
#include <sys/types.h>

// include local
#include "conexao.h"
#include "frame.h"

#define FILE_DESTINATION "./received"

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
  void send_confirm(frame *fReceive);
  void receive_text(frame *f);
  void receive_midia(frame *f);
  frame *receive_frame_socket();
  int receive_valid_frame(frame **f);
  unsigned long chk_available_size();
  long long receive_file_size(frame *f);
  void start_receveing_message();
  bool create_received_dir();
  bool receive_file_name();

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
  vector<char> seq;
  seq.push_back(char(fReceive->get_seq()));
  ack->set_dado(seq);

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
  vector<char> seq;
  seq.push_back(char(fReceive->get_seq()));
  nack->set_dado(seq);

  int nackSent = 0;
  nackSent = socket->send_frame(nack);

  cout << "NACK enviado\n";

  return nackSent;
}

// Recebe uma mensagem em forma de texto
void server::receive_text(frame *f) {
  string textoReceive;
  textoReceive.append(f->get_dado());
  int lastSeq = f->get_seq();

  do {
    if (!receive_valid_frame(&f)) { continue; }
    if (f->get_tipo() != TEXTO) { continue; }
    if (f->get_seq() == lastSeq) { continue; }

    lastSeq = f->get_seq();
    textoReceive.append(f->get_dado());
  } while (f->get_tipo() != FIMT);

  cout << "Mensagem recebida: " << textoReceive << "\n";
}

// Verifica o espaco disponivel em disco
unsigned long server::chk_available_size() {
  struct statvfs st;
  if (statvfs(FILE_DESTINATION, &st) == -1) {
    cout << "Erro no statvfs, abortado\n";
    // send_error();
    return -1;
  }

  return st.f_bsize * st.f_bavail;
}

// Recebe o frame com o tamanho do arquivo
long long server::receive_file_size(frame *f) {
  unsigned long availSize = chk_available_size();
  if (availSize == -1) { return -1; }

  unsigned long fileSize = stoi(f->get_dado());

  if (fileSize > availSize * 0.9) {
    cout << "Tamanho do arquivo muito grande, abortado\n";
    // send_error();
    return -1;
  }

  cout << "Espaco suficiente em disco\n";
  return 1;
}

bool server::create_received_dir() {

  //check if the directory exists
  struct stat info;
  if (stat(FILE_DESTINATION, &info) == 0 && (info.st_mode & S_IFDIR)) {
    cout << "Diretorio ja existe\n";  
    return true;
  }

  //create the directory
  if (mkdir(FILE_DESTINATION, 0700) == -1) {
    cout << "Erro ao criar o diretorio\n";
    return false;
  } 
    
  cout << "Diretorio criado com sucesso\n";
  return true;
}

bool server::receive_file_name() { 
  frame *fReceive;

  // Aguarda receber um frame do tipo midia com o nome do arquivo
  do {
    if (!receive_valid_frame(&fReceive))          
    { 
      cout << "Timeout, falha ao receber o nome do arquivo. Abortado\n";
      return false; 
    }

    if (fReceive->get_tipo() != MIDIA) { continue; }

  } while ( !strncmp(fReceive->get_dado(), "NAME", 4) );
  
  cout << "Nome do arquivo recebido com sucesso\n";
  return true;
 }



// void server::receive_file_data(){
//   vector<frame *> framesMidia;
//   receive

// };

void server::receive_midia(frame *f) {
  if ( !create_received_dir() )   { return; }
  if ( !receive_file_size(f)  )   { return; }
  if ( !receive_file_name()   )   { return; }
  // receive_file_data();
}

// Recebe um frame do cliente
frame *server::receive_frame_socket() {
  frame *fReceive;
  int retries = 0;

  do {
    retries++;
    if (!(fReceive = socket->receive_frame())) { continue; }
  } while (fReceive == NULL && retries < NUM_RETRIES);

  if (fReceive == NULL && retries == NUM_RETRIES) {
    cout << "Desisti de receber o frame\n";
    return NULL;
  }

  return fReceive;
}

int server::receive_valid_frame(frame **f) {
  int crc8;

  do {
    // Se nao conseguir receber o frame, mata a comunicacao
    *f = receive_frame_socket();
    if (*f == NULL) { return 0; }

    // Avisa o cliente se nao conseguiu receber o frame
    crc8 = (*f)->chk_crc8();
    if (crc8) {
      send_ack(*f);
      break;
    }
    send_nack(*f);
  } while (!crc8);

  return 1;
}

void server::start_receveing_message() {
  int endTransmission = 0;

  frame *f;
  do {
    if (!receive_valid_frame(&f)) { return; }
    if (!f) { return; }
    int frameType = f->get_tipo();

    switch (frameType) {
    case TEXTO:
      receive_text(f);
      endTransmission = 1;
      break;

    case MIDIA:
      receive_midia(f);
      endTransmission = 1;
      break;

    default:
      break;
    }
  } while (!endTransmission);
}

// ------------------------------- PUBLIC --------------------------------- //

void server::run() {
  while (true) {

    /*-- listening local ip and waiting for messages --*/
    /*-- ignore if the package is not valid          --*/
    frame *fReceive;
    if (!(fReceive = socket->receive_frame())) { continue; }

    cout << "Frame recebido:" << endl;
    fReceive->imprime(HEX);

    // Verifica se o frame eh de inicio de transmissao e se nao veio com erro
    int frameType = fReceive->get_tipo();
    if (frameType != INIT) { continue; }

    if (!fReceive->chk_crc8()) {
      send_nack(fReceive);
      continue;
    }

    send_ack(fReceive);
    start_receveing_message();
  }
}

#endif
