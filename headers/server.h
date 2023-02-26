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
  int receive_file_size(frame *f);
  void start_receveing_message();
  bool create_received_dir();
  string receive_file_name();
  int receive_file_data(string fileName);
  bool verify_crc8(frame *f);
  bool verify_seq(int seq, int lastSeq);

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
    if (!receive_valid_frame(&f)) { return; }
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
int server::receive_file_size(frame *f) {

  /*
     > verifica a sequencia do frame e o tipo
     > se o frame nao for a sequencia esperada, envia um ack(0) e espera receber
     a sequencia esperada > se o frame for o esperado, verifica o crc8 > se o
     crc estiver certo, envia um ack e continua > se estiver errado, envia um
     nack e espera receber o proximo frame
  */
  do {
    while (f->get_seq() != 1 && f->get_tipo() != MIDIA) {
      f->set_seq(0);
      send_ack(f);
      f = receive_frame_socket();
      if (!f) { return 0; }
    }

    if (verify_crc8(f)) { break; }

    // Se o ack nao estiver certo, espera receber o proximo frame
    f = receive_frame_socket();
    if (!f) { return 0; }
  } while (true);

  unsigned long availSize = chk_available_size();
  if (availSize == -1) { return -1; }

  cout << "Frame file size:" << f->get_dado() << "\n";
  int fileSize = stoi(f->get_dado());

  if (fileSize > availSize * 0.9) {
    cout << "Tamanho do arquivo muito grande, abortado\n";
    // send_error();
    return 0;
  }

  cout << "Espaco suficiente em disco\n";
  return 1;
}

bool server::create_received_dir() {

  // check if the directory exists
  struct stat info;
  if (stat(FILE_DESTINATION, &info) == 0 && (info.st_mode & S_IFDIR)) {
    cout << "Diretorio ja existe\n";
    return true;
  }

  // create the directory
  if (mkdir(FILE_DESTINATION, 0700) == -1) {
    cout << "Erro ao criar o diretorio\n";
    return false;
  }

  cout << "Diretorio criado com sucesso\n";
  return true;
}

string server::receive_file_name() {
  frame *fReceive;

  // Aguarda receber um frame do tipo midia com o nome do arquivo
  // E com a sequencia de numero 2
  /*
    > recebe o frame e verifica se o tipo e a sequencia estao corretos
    > se estiverem, verifica o crc8
    > se estiver correto, envia um ack e continua
    > se estiver errado, envia um nack e espera receber o proximo frame
  */
  do {
    do {
      fReceive = receive_frame_socket();
      if (!fReceive) { return string{}; }

      if (fReceive->get_tipo() != MIDIA) { continue; }
      fReceive->set_seq(1);
      send_ack(fReceive);

    } while (fReceive->get_seq() != 2);

    if (verify_crc8(fReceive)) { break; }

    fReceive = receive_frame_socket();
    if (!fReceive) { return string{}; }

  } while (true);

  cout << "Nome do arquivo recebido com sucesso\n";
  cout << "Nome do arquivo: " << fReceive->get_dado() << "\n";

  return string(fReceive->get_dado());
}

bool server::verify_seq(int seq, int lastSeq) {
  if (lastSeq == 15 && seq != 0) { return false; }
  if (seq != lastSeq + 1) { return false; }
  return true;
}

int server::receive_file_data(string fileName) {
  cout << "\tRecebendo dados arquivo\n";
  // Abre o arquivo para escrita
  ofstream file;
  file.open(fileName, ios::binary);
  if (!file.is_open()) {
    cout << "Falha ao criar o arquivo. Abortado\n";
    return 0;
  }

  cout << "Arquivo criado com sucesso\n";

  int lastSeq = 2;
  frame *f;

  do {
    // Fica tentando receber um frame
    f = receive_frame_socket();
    if (f == NULL) { return 0; }

    if (f->get_tipo() == FIMT) { break; }

    if (f->get_tipo() != DADOS) { continue; }

    // Recebeu um frame com uma sequencia errada
    if (!verify_seq(f->get_seq(), lastSeq)) {
      f->set_seq(lastSeq);
      send_ack(f);
      continue;
    }

    if (!verify_crc8(f)) { continue; }

    lastSeq = f->get_seq();
    file.write(f->get_dado(), f->get_tam());

  } while (true);

  cout << "Arquivo recebido com sucesso\n";

  file.close();
  return 1;
}

void server::receive_midia(frame *f) {
  if (!create_received_dir()) { return; }
  if (!receive_file_size(f)) { return; }

  string fileName = receive_file_name();

  if (fileName.size() == 0) { return; }

  receive_file_data(fileName);
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

bool server::verify_crc8(frame *f) {
  int crc8 = f->chk_crc8();
  if (!crc8) {
    send_nack(f);
  }
  else
    send_ack(f);
  return crc8;
}

int server::receive_valid_frame(frame **f) {

  do {
    // Se nao conseguir receber o frame, mata a comunicacao
    *f = receive_frame_socket();
    if (*f == NULL) { return 0; }

    cout << "Frame recebido loop\n";
    (*f)->imprime(HEX);
    // Avisa o cliente se nao conseguiu receber o frame
  } while (!verify_crc8(*f));
  return 1;
}

void server::start_receveing_message() {
  int continueTransmission = 1;

  frame *f;
  do {
    if (!receive_valid_frame(&f)) { return; }
    if (!f) { return; }
    int frameType = f->get_tipo();

    switch (frameType) {
    case TEXTO:
      receive_text(f);
      continueTransmission = 0;
      break;

    case MIDIA:
      receive_midia(f);
      continueTransmission = 0;
      break;

    default:
      break;
    }
  } while (continueTransmission);
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
