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
#include "cores.h"
#include "frame.h"
#include "macros.h"
#include "conexao.h"

#define FILE_DESTINATION "./received"

using namespace std;

class server {

private:
  // --------- Dados ---------- //
  //int soquete;
  int tipoReceivingFrames;

  conexao *socket;

  //vector<frame *> framesMidia;

  // ---------- Funcoes -------- //

  frame*    create_ack_nack     (int tipo, int seq);
  int       send_nack           (frame *fReceive);
  int       send_ack            (frame *fReceive);
  bool      verify_seq          (int seq, int lastSeq);
  int       next_tipo_midia     (frame *f);
  bool      create_received_dir ();
  UL        chk_available_size  ();
  int       receive_file_size   (frame *f);
  string    create_file_destination(string fileName);
  ofstream  create_file         (string fileName);
  int       receive_midia       (frame *f);
  //void send_confirm(frame *fReceive);
  //void receive_text(frame *f);
  //int receive_valid_frame(frame **f);
  //int receive_file_data(string fileName);
  //bool verify_crc8(frame *f);
  frame *receive_frame_socket();
  queue<frame *> receive_frames_window(int lastSeq);
  void start_receveing_message();

public:
  // ------- Construtores ------ //
  server(conexao *socketAddr) { socket = socketAddr; };

  // ---------- Funcoes -------- //
  void run();
};

// ------------------------------ PRIVATE --------------------------------- //
/**
 * @brief function that creates a nack or a nack 
 *
 * @param tipo ack or nack
 * @param seq  ack or nack sequence number
 * @return int
 */

frame *server::create_ack_nack(int tipo, int seq)
{
  vector<char> seq_char;
  frame *f = new frame();

  f->set_tipo(tipo);
  vector<char> seq_v;
  seq_v.push_back(char(seq));
  f->set_dado(seq_v);

  return f;
}

/**
 * @brief function that sends a ack frame to the target
 *
 * @param fReceive frame received
 * @return int
 */
int server::send_ack(frame *fReceive) {

  frame *ack = create_ack_nack(ACK, fReceive->get_seq());

  int ackSent = socket->send_frame(ack);
  if ( ackSent == -1 ) 
  { 
    cout << "Falha ao enviar o ack\n";
    return -1; 
  }

  cout << "ACK " << (int) fReceive->get_seq() << " enviado\n";

  return ackSent;
}

/**
 * @brief function that sends a nack frame to the target
 *
 * @param fReceive frame received
 * @return int
 */
int server::send_nack(frame *fReceive) {

  frame *nack = create_ack_nack(NACK, fReceive->get_seq());

  int nackSent = socket->send_frame(nack);
  if ( nackSent == -1 ) { 
    cout << "Falha ao enviar o nack";
    return -1; 
  }

  cout << "NACK " << (int) fReceive->get_seq() << " enviado\n";

  return nackSent;
}

/**
 * @brief function that verify if two numbers are sequential
 *
 * @param seq
 * @param lastSeq
 * @return bool 
 */
bool server::verify_seq(int seq, int lastSeq) {
  if ( seq == 0 )
  {
    if ( lastSeq != 15 ) { return false; }
    return true;
  }

  if (seq != lastSeq + 1) { return false; }
  return true;
}

int server::next_tipo_midia(frame *f) {
  if ( f->get_tipo() != MIDIA ) { return -1; }
  if ( f->get_seq() == 0 )      { return MIDIA; }
  if ( f->get_seq() == 1 )      {
    cout << YELLOW << "\t--Recebendo dados arquivo--\n" << RESET;
    return DADOS; }

  return -1;
}

//// Recebe uma mensagem em forma de texto
//void server::receive_text(frame *f) {
//  string textoReceive;
//  textoReceive.append(f->get_dado());
//  int lastSeq = f->get_seq();
//
//  do {
//    if (!receive_valid_frame(&f)) { return; }
//    if (f->get_tipo() != TEXTO) { continue; }
//    if (f->get_seq() == lastSeq) { continue; }
//
//    lastSeq = f->get_seq();
//    textoReceive.append(f->get_dado());
//  } while (f->get_tipo() != FIMT);
//
//  cout << "Mensagem recebida: " << textoReceive << "\n";
//}
//
//// Verifica o espaco disponivel em disco

UL server::chk_available_size() {
  struct statvfs st;
  if (statvfs(FILE_DESTINATION, &st) == -1) {
    // cout << "Erro no statvfs, abortado\n";->log
    // send_error();
    return -1;
  }

  return st.f_bsize * st.f_bavail;
}

//// Recebe o frame com o tamanho do arquivo
int server::receive_file_size(frame *f) {

 cout << "Recebendo tamanho do frame\n";
 unsigned long availSize = chk_available_size();
 if (availSize == -1) { return -1; }

 cout << "Frame file size:" << f->get_dado() << "\n";
 int fileSize = stoi(f->get_dado());

 if (fileSize > availSize * 0.9) {
    cout << BOLDMAGENTA << "\t--Tamanho do arquivo muito grande, abortado--\n"
         << RESET;
    // send_error();
    return 0;
 }

  // cout << "Espaco suficiente em disco\n"; ->log
  return 1;
}

bool server::create_received_dir() {

  cout << "Criando diretorio";
  // Check if the directory exists
  struct stat info;
  if (stat(FILE_DESTINATION, &info) == 0 && (info.st_mode & S_IFDIR)) {
    // cout << "Diretorio ja existe\n"; ->log
    return true;
  }

  // create the directory
  if (mkdir(FILE_DESTINATION, 0777) == -1) {
    cout << RED << "Erro ao criar o diretorio\n" << RESET;
    return false;
  }

  // cout << "Diretorio criado com sucesso\n"; ->log
  return true;
}

//int server::receive_file_data(string fileName) {
//
//  int lastSeq = 1;
//  frame *f;
//
//  cout << "\tRecebendo dados arquivo\n";
//  do {
//    cout << "Aguardando frame\n";
//
//    // Fica tentando receber um frame
//    f = receive_frame_socket();
//    if (f == NULL) { return 0; }
//
//    cout << "Frame recebido\n";
//    f->imprime(HEX);
//
//    if (f->get_tipo() == FIMT) { break; }
//
//    if (f->get_tipo() != DADOS) { continue; }
//
//    if (f->get_seq() == lastSeq) { continue; }
//
//    // Recebeu um frame com uma sequencia errada
//    if (!verify_seq(f->get_seq(), lastSeq)) {
//      cout << "Frame com a sequencia errada; Pedindo a certa\n";
//      f->set_seq(lastSeq);
//      send_ack(f);
//      continue;
//    }
//
//    if (!verify_crc8(f)) { 
//      cout << "Crc invalido\n";
//      continue; 
//    }
//
//    cout << "Seq " << int(f->get_seq()) << "recebida com sucesso\n";
//    lastSeq = f->get_seq();
//    file.write(f->get_dado(), f->get_tam());
//
//  } while (true);
//
//  cout << "Dados do arquivo recebido com sucesso\n";
//  send_ack(f);
//
//  file.close();
//  return 1;
//}
//

string server::create_file_destination(string fileName)
{
  string fileDestination;
  fileDestination.append(FILE_DESTINATION);
  fileDestination.push_back('/');
  fileDestination.append(fileName);

  return fileDestination;
}

ofstream server::create_file(string fileName) {
  string fileDestination = create_file_destination(fileName);

  cout << BOLDWHITE << "Criando arquivo " << BOLDYELLOW << fileDestination
       << BOLDWHITE << ". Digite novo nome ou enter para continuar: " << RESET;

  string newDestination = "";
  getline(cin, newDestination);
  if (!newDestination.empty() && newDestination != "\n") {
    fileDestination = newDestination;
  }
  cout << YELLOW << "\t--Criando arquivo: " << fileDestination << "--\n"
       << RESET;

  // Abre o arquivo para escrita
  ofstream file;
  file.open(fileDestination, ios::binary);
  return file;
}

int server::receive_midia(frame *f) {
  
  ofstream file;

  // Escreve o dado no arquivo
  if ( f->get_tipo() == DADOS )
  {
    file.write(f->get_dado(), f->get_tam());
    return 1;
  }

  // Primeiro frame de midia
  if ( f->get_seq() == 0 )
  {
    cout << "Recebendo tamanho do arquivo\n";
    if (!create_received_dir()) { return 0; }
    if (!receive_file_size(f))  { return 0; }
  }

  // Segundo frame de midia
  string fileName = string(f->get_dado());
  file = create_file(fileName);

  if (!file.is_open())
  {
    cout << RED << "\tFalha ao criar o arquivo. Abortado\n" << RESET;
    file.close();
    remove(create_file_destination(fileName).c_str());
    return 0;
  }

  // cout << BOLDGREEN << "\t--Arquivo recebido com sucesso--\n" << RESET;
  if ( !file.is_open() )     { return 0; }
  return 1;
}

//// Recebe um frame do cliente
frame *server::receive_frame_socket() {
  frame *fReceive;
  int retries = 0;

  do {
    retries++;
    if (!(fReceive = socket->receive_frame())) { continue; }
  } while (fReceive == NULL && retries < NUM_RETRIES);

  if (fReceive == NULL && retries == NUM_RETRIES) {
    // cout << "Desisti de receber o frame\n"; ->log
    return NULL;
  }

  return fReceive;
}
//
//bool server::verify_crc8(frame *f) {
//  int crc8 = f->chk_crc8();
//  if (!crc8) {
//    send_nack(f);
//  }
//  else
//    send_ack(f);
//  return crc8;
//}
//
//int server::receive_valid_frame(frame **f) {
//
//  do {
//    // Se nao conseguir receber o frame, mata a comunicacao
//    *f = receive_frame_socket();
//    if (*f == NULL) { return 0; }
//
//    // Avisa o cliente se nao conseguiu receber o frame
//  } while (!verify_crc8(*f));
//  return 1;
//}

queue<frame *> server::receive_frames_window(int lastSeq)
{
  queue<frame *> frames_queue;
  frame *f;
  int retries = 0;

  do {
    if ( ! (f = receive_frame_socket()) ) { continue; }
    
    retries++;

    int tipo = f->get_tipo();

    // Adiciona o frame de fim de transmissao
    if ( tipo == FIMT ) 
    {
      frames_queue.push(f);
      return frames_queue;
    }

    // Primeiro frame a ser recebido, seta o tipo
    if ( lastSeq == -1 ) 
    {
      // Ignora os frames perdidos na linha
      if ( (tipo != MIDIA && tipo != TEXTO) || f->get_seq() != 0 ) {continue;}
      tipoReceivingFrames = f->get_tipo();
      frames_queue.push(f);
      lastSeq = 0;
      retries = 0;
      continue; 
    }


    // Ignora se o frame nao for do tipo midia e esteja recebendo midia
    if ( tipo == MIDIA && tipoReceivingFrames == MIDIA )
    {
      // Ignora se for um frame do tipo midia que nao e o segundo da sequencia
      if ( lastSeq != 0 || (TAM_JANELA > 1 && f->get_seq() != 1)) { continue; }

      // Se for o segundo frame do tipo midia, muda o tipo esperado
      tipoReceivingFrames = DADOS;
      frames_queue.push(f);
      lastSeq = 1;
      retries = 0;
      continue;
    }

    // Recebe os frames de dados de um arquivo
    if ( tipoReceivingFrames == DADOS && tipo == DADOS )
    {
      if ( !verify_seq(f->get_seq(), lastSeq) ) { continue; }
      frames_queue.push(f);
      retries = 0;
      lastSeq = f->get_seq();
      continue;
    }

    // Recebe os frames de uma mensagem
    if ( tipoReceivingFrames == TEXTO && tipo == TEXTO )
    {
      if ( !verify_seq(f->get_seq(), lastSeq) ) { continue; }
      frames_queue.push(f);
      retries = 0;
      lastSeq = f->get_seq();
      continue;
    }

  } while ( (f == NULL && retries < NUM_RETRIES) || frames_queue.size() < TAM_JANELA );

  if ( f == NULL && retries == NUM_RETRIES ) { return queue<frame *>(); }

  return frames_queue;
}

void server::start_receveing_message() {
  int continueTransmission = 1;
  int lastSeq = -1;
  int tipo_data = -1;
  vector<char> data;
  queue<frame *> client_answer;

  cout << "Recebendo frames\n";
  // Fic aouvindo o cliente ate receber um FIMT
  do {
    queue<frame *> frames = receive_frames_window(lastSeq);
    if ( frames.empty() ) { return; }

    cout << "Quantidade de frames na janela: " << frames.size() << "\n";

    // Ve o que faz com cada frame de acordo com o tipo
    while ( !frames.empty() )
    {
      frame *f = frames.front();
      frames.pop();

      // Recebeu um frame com erro, retorna um nack e sai da funcao
      if ( !f->chk_crc8() ) {
        client_answer.push(create_ack_nack(NACK, f->get_seq()));
        continue;
      }

      else { 
        client_answer.push(create_ack_nack(ACK, f->get_seq()));
      }
      
      cout << "Frame recebido: \n";
      f->imprime(HEX);
      cout << "\n";

      int tipo = f->get_tipo();
      int tam = f->get_tam();
      char *data_f = f->get_dado();

      switch (tipo)
      {
        case FIMT:
          cout << "Encerrou a transmissao\n";
          continueTransmission = 0;
          break;

        case TEXTO:
          data.insert(data.end(), data_f, data_f + tam);
          lastSeq = f->get_seq();
          tipo_data = TEXTO;
          break;

        case MIDIA:
          if ( !receive_midia(f) ) { return; }
          lastSeq = f->get_seq();
          tipo_data = next_tipo_midia(f);
          break;

        case DADOS:
          if ( !receive_midia(f)) { return; }
          lastSeq = f->get_seq();
          tipo_data = DADOS;
          break;

        default:
          break;
      }
    }

    cout << "Recebeu todos os frames de uma janela\n";

    // Envia a reposta ao cliente
    cout << "Enviando acks e nacks para o cliente\n";
    while ( !client_answer.empty() )
    {
      frame *f_answer = client_answer.front();
      client_answer.pop();

      if (socket->send_frame(f_answer) == -1 )
      {
        cout << "Falha ao enviar a resposta\n";
        return;
      }

      if (f_answer->get_tipo() == NACK)
        cout << "NACK " << (int)f_answer->get_dado()[0] << " enviado\n";

      else
        cout << "ACK " << (int)f_answer->get_dado()[0] << " enviado\n";
    }

    cout << "Todos os ACKs e NACKs foram enviados\n";

  } while (continueTransmission);

  if ( tipo_data == TEXTO )
    cout << BOLDYELLOW << "\t--Mensagem recebida--\n " << BOLDWHITE
         << string(data.begin(), data.end()) << "\n"
         << RESET;

  if ( tipo_data == DADOS )
    cout << BOLDGREEN << "\t--Arquivo recebido com sucesso--\n" << RESET;
}

// ------------------------------- PUBLIC --------------------------------- //

void server::run() {
  while (true) {

    /*-- listening local ip and waiting for messages --*/
    /*-- ignore if the package is not valid          --*/
    frame *fReceive;
    if (!(fReceive = socket->receive_frame())) { continue; }

    // cout << "Frame recebido:" << endl; ->log
    // fReceive->imprime(HEX);

    // Verifica se o frame eh de inicio de transmissao e se nao veio com erro
    int frameType = fReceive->get_tipo();
    if (frameType != INIT) {
      // cout << "Frame ignorado, n eh INIT\n"; ->log
      continue;
    }

    // cout << "Frame de INIT\n"; ->log
    if (!fReceive->chk_crc8()) {
      send_nack(fReceive);
      continue;
    }

    if ( send_ack(fReceive) == -1 ) { continue; }
    start_receveing_message();
  }
}

#endif
