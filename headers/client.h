// Receive all the data
#ifndef _CLIENT_
#define _CLIENT_

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
#include <sys/stat.h>
#include <sys/types.h>

// include local
#include "conexao.h"
#include "cores.h"
#include "frame.h"
using namespace std;

class client {

private:
  // --------- Dados ---------- //
  int soquete;
  vector<frame *> framesMidia;
  conexao *socket;

  // ---------- Funcoes -------- //
  int send_frames(vector<frame *> frames);
  int send_message(vector<char> data, int type);
  void send_file();
  void send_text(string message);
  bool string_has(string str, vector<string> strList);
  char string_cmd(string str);
  void print_help();
  bool verify_ack(frame *received, frame *sent);
  frame *receive_ack(frame *f);
  vector<frame *> create_frames(vector<char> data, int type);
  vector<frame *> create_frames_midia(vector<char> data);
  int send_frame_socket(frame *f);
  int start_transmission();
  int end_transmission();
  string calc_file_size(string fileName);
  vector<char> read_file(string fileName);

public:
  // --------- Dados ---------- //
  string userInput;

  // ------- Construtores ------ //
  client(conexao *socketAddr) { socket = socketAddr; }

  // ---------- Funcoes -------- //
  void run();
};

// ------------------------------ PRIVATE --------------------------------- //

// Verifica se recebeu um ack valido
frame *client::receive_ack(frame *f) {
  frame *ack_res = NULL;

  // se recebemos algo, e NÃO ẽ o ACK que estamos
  // esperando, continuamos tentando receber
  do {
    ack_res = socket->receive_frame();
    if (ack_res && ack_res->get_tipo() == ERRO) {
      cout << BOLDYELLOW << "Espaco insulficiente no destino\n" << RESET;
      return NULL;
    }
  } while (ack_res &&
           !(verify_ack(ack_res, f) && ack_res->get_dado()[0] == f->get_seq()));

  return ack_res;
}

// Solicita ao socket que envie um frame
int client::send_frame_socket(frame *f) {
  // Fica tentando enviar o frame até receber o ack
  frame *ack_res = NULL;
  int retries = 0;
  do {
    // envia um frame da fila
    socket->send_frame(f);
    ack_res = receive_ack(f);
    retries++;
  } while (ack_res == NULL && retries < NUM_RETRIES);

  if (ack_res == NULL && retries == NUM_RETRIES) {
    // cout << "Desisti de enviar\n"; ->log
    return 0;
  }

  // cout << "\tACK recebido:\n"; ->log
  // ack_res->imprime(HEX);
  return 1;
}

// Inicia a transmissao com o servidor
int client::start_transmission() {
  // cout<< YELLOW << "\tIniciando transmissao\n" << RESET; ->log
  frame *ini = new frame(INIT, 0, vector<char>(1, 0));
  int enviado = send_frame_socket(ini);
  if (!enviado) {
    // cout<< BOLDRED << "\tFalha ao iniciar a transmissao\n" << RESET; ->log
    return 0;
  }

  // cout<< GREEN << "\tTransmissao iniciada com sucesso\n" << RESET; ->log
  return 1;
}

// Encerra a transmissao com o servidor
int client::end_transmission() {
  // cout << "\tEncerrando a transmissao\n"; ->log
  frame *end = new frame(FIMT, 0, vector<char>(1, 0));
  int enviado = send_frame_socket(end);
  if (!enviado) {
    // cout << "\tFalha ao encerrar a transmissao\n"; ->log
    return 0;
  }

  // cout << "\tTransmissao encerrada com sucesso\n"; ->log
  return 1;
}

/**
 * @brief Send a list of frames through the socket
 *
 * @param f
 * @return int
 */
int client::send_frames(vector<frame *> frames) {
  if (frames.empty()) { return 0; }

  if (!start_transmission()) { return 0; }

  // Envia um frame por vez
  for (size_t i = 0; i < frames.size(); i++) {

    // cout << "\tEnviando frame\n"; ->log
    // frames[i]->imprime(DEC);

    int enviado = send_frame_socket(frames[i]);
    if (!enviado) {
      // cout << "\tFalha ao enviar o frame\n"; ->log
      return 0;
    }

    // cout << "\tFrame enviado com sucesso\n"; ->log
  }

  if (!end_transmission()) { return 0; }
  // cout << "\tTerminou de enviar todos os frames\n"; ->log
  return 1;
}

/**
 * @brief verify if the received frame is an ACK and if it is the same as the
 * sent frame
 *
 * @param received
 * @param sent
 * @return true
 * @return false
 */
bool client::verify_ack(frame *received, frame *sent) {
  return (received->get_tipo() == ACK && received->chk_crc8());
}

/**
 * @brief Send data through the socket
 *
 * @param data: data to be sent
 * @return int
 */
int client::send_message(vector<char> data, int type) {

  int messageSize = data.size();
  vector<frame *> frames;

  switch (type) {
  case TEXTO:
    frames = create_frames(data, type);
    break;

  case MIDIA:
    frames = create_frames_midia(data);
    break;
  }

  return send_frames(frames);
}

string client::calc_file_size(string fileName) {
  struct stat buffer;
  if (stat(fileName.c_str(), &buffer) == -1) {
    cout << YELLOW << "\t--Arquivo inexistente. Operacao abortada--\n" << RESET;
    return {};
  }

  long long fileSize = buffer.st_size;
  return to_string(fileSize);
}

vector<char> client::read_file(string fileName) {
  ifstream file;
  file.open(fileName, ios::binary);

  vector<char> fileData;
  for (char c; (file.read(&c, 1), file.eof()) == false; fileData.push_back(c))
    ;

  file.close();
  return fileData;
}

/**
 * @brief Send a file through the socket
 *
 */
void client::send_file() {
  vector<char> fileNameVector;
  string fileName;

  do {
    cout << BOLDWHITE << "Digite o nome do arquivo(maximo de " << TAM_DADOS
         << " char):\n"
         << RESET;
    getline(cin, fileName);
  } while (fileName.size() > TAM_DADOS);

  fileNameVector.insert(fileNameVector.begin(), fileName.begin(),
                        fileName.end());

  cout << BOLDYELLOW << "\t--Enviando arquivo--\n" << RESET;
  if (!send_message(fileNameVector, MIDIA)) {
    cout << BOLDRED << "\t--Falha ao enviar o arquivo--\n" << RESET;
    return;
  }

  cout << BOLDGREEN << "\t--Arquivo enviado com sucesso--\n" << RESET;
}

/**
 * @brief Send a text message through the socket
 *
 * @param message
 */
void client::send_text(string message) {

  cout << BOLDYELLOW << "\t--Enviando mensagem--\n" << RESET;

  vector<char> data(message.begin(), message.end());
  if (!send_message(data, TEXTO))
    cout << BOLDRED << "\t--Limite de timout, mensagem nao foi enviada--\n"
         << RESET;
  else
    cout << GREEN << "\t--Mensagem enviada com sucesso--\n" << RESET;
}

vector<frame *> client::create_frames_midia(vector<char> vectorName) {
  vector<frame *> framesToSend;
  vector<frame *> framesAux;

  // Cria um vetor com o tamanho do arquivo
  vector<char> vectorTam;
  string fileName = string(vectorName.begin(), vectorName.end());
  string fileSize = calc_file_size(fileName);
  if (fileSize.empty()) { return vector<frame *>(); }
  vectorTam.insert(vectorTam.begin(), fileSize.begin(), fileSize.end());

  // Adiciona o tamanho no primeiro frame a ser enviado
  framesAux = create_frames(vectorTam, MIDIA);
  framesToSend.insert(framesToSend.end(), framesAux.begin(), framesAux.end());

  // Adiciona o nome no segundo frame a ser enviado
  framesAux = create_frames(vectorName, MIDIA);
  framesToSend.insert(framesToSend.end(), framesAux.begin(), framesAux.end());

  // Cria um vetor com os dados do arquivo
  vector<char> vectorData = read_file(fileName);
  if (vectorData.empty()) {
    cout << BOLDRED << "\t--Falha ao abrir o arquivo para leitura--\n"
         << RESET; //->log
    return vector<frame *>();
  }

  // Adiciona os dados do arquivo
  framesAux = create_frames(vectorData, DADOS);
  framesToSend.insert(framesToSend.end(), framesAux.begin(), framesAux.end());

  // Arruma a sequencia dos frames
  for (int i = 0; i < framesToSend.size(); i++)
    framesToSend[i]->set_seq(i % 16);

  return framesToSend;
}

/**
 * @brief Creates a vector with all the frames to be sent
 *
 * @param message
 */

vector<frame *> client::create_frames(vector<char> data, int type) {
  vector<frame *> frames;
  int i = 0;

  int frameCnt = (data.size() / TAM_DADOS) + bool(data.size() % TAM_DADOS);
  while (i < frameCnt) {
    frame *f = new frame();
    f->set_tipo(type);
    f->set_seq(i);
    f->set_dado(vector<char>(
        data.data() + i * TAM_DADOS,
        data.data() + min(data.size(), (size_t)(i + 1) * TAM_DADOS)));
    frames.push_back(f);
    i++;
  }

  return frames;
}

bool client::string_has(string str, vector<string> strList) {

  for (int i = 0; i < strList.size(); i++) {
    if (str.find(strList[i]) != string::npos) { return true; }
  }

  return false;
}

char client::string_cmd(string str) {
  if (strcmp(str.c_str(), "-h") == 0) return 'h';
  if (strcmp(str.c_str(), "-e") == 0) return 'e';
  if (strcmp(str.c_str(), "-s") == 0) return 's';
  return 'm';
}

void client::print_help() {
  cout << BOLDBLUE << "\t---Comandos disponiveis---\n" << RESET;
  cout << YELLOW
       << "para enviar uma mensagem, digite a mensagem e pressione enter\n"
       << RESET;
  cout << GREEN << "-s: " << WHITE << "envia um arquivo\n" << RESET;
  cout << RED << "-e: " << WHITE << "sai do programa\n" << RESET;
  cout << BLUE << "-h: " << WHITE << "exibe os comandos disponíveis\n" << RESET;
}

// ------------------------------- PUBLIC --------------------------------- //

void client::run() {
  int i;
  print_help();
  while (true) {
    cout << BOLDWHITE << "\nDigite um comando ou mensagem:\n" << RESET;

    getline(cin, userInput);
    char userInputCMD = string_cmd(userInput);

    switch (userInputCMD) {
    case 'h':
      print_help();
      break;

    case 'e':
      cout << BOLDYELLOW << "\tSaindo...\n" << RESET;
      exit(0);
      break;

    case 's':
      send_file();
      break;

    case 'm':
      send_text(userInput);
      break;

    default:
      cout << BOLDMAGENTA << "\tComando invalido\n" << RESET;
      break;
    }
  }
}

#endif
