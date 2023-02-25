// Receive all the data
#ifndef _CLIENT_
#define _CLIENT_

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
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

// include local
#include "conexao.h"
#include "frame.h"

vector<string> CMD_HELP = {"H",  "help",  "-h",    "HELP", "Help",
                           "-H", "ajuda", "Ajuda", "AJUDA"};
vector<string> CMD_EXIT = {"exit", "-e",   "EXIT", "Exit",
                           "-E",   "sair", "Sair", "SAIR"};
vector<string> CMD_SEND = {"send", "-s",     "SEND",   "Send",
                           "-S",   "enviar", "Enviar", "ENVIAR"};

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
  int send_frame_socket(frame *f);
  int start_transmission();
  int end_transmission();
  string calc_file_size(string fileName);
  vector<char> read_file(string fileName);

public:
  // --------- Dados ---------- //
  string userInput;

  // ------- Construtores ------ //
  client(conexao *socketAddr) { socket = socketAddr; };

  // ---------- Funcoes -------- //
  void run();
};

// ------------------------------ PRIVATE --------------------------------- //

// Verifica se recebeu um ack valido
frame *client::receive_ack(frame *f)
{
  frame *ack_res = NULL;

  // se recebemos algo, e NÃO ẽ o ACK que estamos
  // esperando, continuamos tentando receber
  do {
    ack_res = socket->receive_frame();
    if (ack_res->get_tipo() == ERRO) 
    { 
      cout << "Espaco insulficiente no destino\n";
      return NULL;
    }

  } while (
    ack_res &&
    !(verify_ack(ack_res, f) &&
      ack_res->get_dado()[0] == f->get_seq())
  );

  return ack_res;
}

// Solicita ao socket que envie um frame
int client::send_frame_socket(frame *f)
{
  // Fica tentando enviar o frame até receber o ack
  frame* ack_res = NULL;
  int retries = 0;
  do {
    // envia um frame da fila
    socket->send_frame(f);
    ack_res = receive_ack(f);
    retries++;

  } while (ack_res == NULL && retries < NUM_RETRIES);

  if (ack_res == NULL && retries == NUM_RETRIES) {
    cout << "Desisti de enviar\n";
    return 0;
  }

  cout << "\tACK recebido:\n";
  ack_res->imprime(HEX);
  return 1;
}

// Inicia a transmissao com o servidor
int client::start_transmission()
{
  cout << "\tIniciando transmissao\n";
  frame *ini = new frame(INIT, 0, vector<char>(1,0));
  int enviado = send_frame_socket(ini);
  if ( !enviado )
  {
    cout << "\tFalha ao iniciar a transmissao\n";
    return 0;
  }

  cout << "\tTransmissao iniciada com sucesso\n";
  return 1;
}

// Encerra a transmissao com o servidor
int client::end_transmission()
{
  cout << "\tEncerrando a transmissao\n";
  frame *end= new frame(FIMT, 0, vector<char>(1,0));
  int enviado = send_frame_socket(end);
  if ( !enviado )
  {
    cout << "\tFalha ao encerrar a transmissao\n";
    return 0;
  }

  cout << "\tTransmissao encerrada com sucesso\n";
  return 1;
}

/**
 * @brief Send a list of frames through the socket
 *
 * @param f
 * @return int
 */
int client::send_frames(vector<frame *> frames) {
  if ( !start_transmission() ) { return 0; }

  // Envia um frame por vez
  for (size_t i = 0; i < frames.size(); i++) {

    cout << "\tEnviando frame\n";
    frames[i]->imprime(DEC);

    int enviado = send_frame_socket(frames[i]);
    if ( !enviado ) 
    {
      cout << "\tFalha ao enviar o frame\n";
      return 0;
    }

    cout << "\tFrame enviado com sucesso\n";
  }

  if ( !end_transmission() ) { return 0; }
  cout << "\tTerminou de enviar todos os frames\n";
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
  return ( received->get_tipo() == ACK && received->chk_crc8());
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
  frames = create_frames(data, type);

  return send_frames(frames);
}

string client::calc_file_size(string fileName)
{
  struct stat buffer;
  stat(fileName.c_str(), &buffer);
  int fileSize = buffer.st_size;

  return to_string(fileSize);
}

vector<char> client::read_file(string fileName)
{
  fstream file;
  file.open(fileName, ios::in); 

  if (!file) {
    cout << "Arquivo inexistente. Operacao abortada\n";
    return vector<char>();
  }
  
  string teste;
  vector<char> fileData;
  char c;
  while ( (file.get(c), file.eof() == false) )
  {
    fileData.push_back(c);
    teste.push_back(c);
  }

  file.close();
  cout << "vetor criado: " << teste << "\n";
  return fileData;
}

/**
 * @brief Send a file through the socket
 *
 */
void client::send_file() {
  string fileName;
  do {
    cout << "Digite o nome do arquivo(maximo de " << TAM_DADOS << " char):\n";
    getline(cin, fileName);
  } while ( fileName.size() > TAM_DADOS );

  // Envia o primeiro frame com o tamanho do arquivo
  string fileSize = calc_file_size(fileName);
  cout << "Tamanho do arquivo: " << fileSize << "\n";
  cout << "Enviando tamanho do arquivo\n";
  if (!send_message(vector<char>(fileSize.begin(), fileSize.end()), MIDIA))
  {
    cout << "Limite de timout, arquivo nao foi enviado\n";
    return;
  }

  // Envia o segundo frame com o nome do arquivo
  cout << "Enviando nome do arquivo\n";
  if (!send_message(vector<char>(fileName.begin(), fileName.end()), MIDIA))
  {
    cout << "Limite de timout, arquivo nao foi enviado\n";
    return;
  }

  cout << "Enviando arquivo\n";
  vector<char> file = read_file(fileName);
  if (file.empty() || !send_message(file, DADOS))
  {
    cout << "Limite de timout, arquivo nao foi enviado\n";
    return;
  }

  else
    cout << "Arquivo enviado com sucesso\n";
}

/**
 * @brief Send a text message through the socket
 *
 * @param message
 */
void client::send_text(string message) {

  cout << "Enviando mensagem\n";

  vector<char> data (message.begin(), message.end());
  if (!send_message(data, TEXTO))
    cout << "Limite de timout, mensagem nao foi enviada\n";

  else
    cout << "Mensagem enviada com sucesso\n";
}

/**
 * @brief Creates a vector with all the frames to be sent
 *
 * @param message
 */

vector<frame *> client::create_frames(vector<char> data, int type) {
  vector<frame *> frames;
  int i = 0;

  int frameCnt = (data.size()/TAM_DADOS) + bool(data.size()%TAM_DADOS);
  while (i < frameCnt) {
    frame *f = new frame();
    f->set_tipo(type);
    f->set_seq(i);
    f->set_dado(vector<char>(data.data()+i*TAM_DADOS, data.data()+
                             min(data.size(), (size_t)(i+1)*TAM_DADOS)));
    frames.push_back(f);
    i++;
  }

  return frames;
}

bool client::string_has(string str, vector<string> strList) {

  for (int i = 0; i < strList.size(); i++) {
    if (str.find(strList[i]) != string::npos) {
      return true;
    }
  }

  return false;
}

char client::string_cmd(string str) {
  if (string_has(str, CMD_HELP))
    return 'h';

  if (string_has(str, CMD_EXIT))
    return 'e';

  if (string_has(str, CMD_SEND))
    return 's';

  return 'm';
}

void client::print_help() {
  cout << "Comandos disponiveis:\n";
  cout << "help ou -h ou h ou HELP ou Help ou H ou ajuda ou Ajuda ou AJUDA\n";
  cout << "exit ou -e ou e ou EXIT ou Exit ou E ou sair ou Sair ou SAIR\n";
  cout
      << "send ou -s ou s ou SEND ou Send ou S ou enviar ou Enviar ou ENVIAR\n";
  cout << "para enviar uma mensagem, digite a mensagem e pressione enter\n";
}

// ------------------------------- PUBLIC --------------------------------- //

void client::run() {
  int i;
  while (true) {
    cout << " Digite um comando ou mensagem:\n";

    getline(cin, userInput);
    char userInputCMD = string_cmd(userInput);

    switch (userInputCMD) {
    case 'h':
      print_help();
      break;

    case 'e':
      cout << "Saindo...\n";
      exit(0);
      break;

    case 's':
      send_file();
      break;

    case 'm':
      send_text(userInput);
      break;

    default:
      cout << "Comando invalido\n";
      break;
    }
  }
}

#endif
