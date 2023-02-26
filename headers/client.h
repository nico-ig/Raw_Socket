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
  bool verify_ack_nack(frame *received);
  frame *receive_ack_nack();
  vector<frame *> create_frames(vector<char> data, int type);
  vector<frame *> create_frames_midia(vector<char> data);
  frame* send_frame_socket(frame *f);
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
frame *client::receive_ack_nack() {
  frame *response = NULL;

  // se recebemos algo, e NÃO ẽ o ACK que estamos
  // esperando, continuamos tentando receber
  do {
    response = socket->receive_frame();
    if (response && response->get_tipo() == ERRO) {
      cout << "Espaco insulficiente no destino\n";
      return NULL;
    }
  } while (response &&
           !(verify_ack_nack(response)));

  return response;
}

// Solicita ao socket que envie um frame
frame* client::send_frame_socket(frame *f) {
  // Fica tentando enviar o frame até receber o ack
  frame *response = new frame();
  int retries = 0;
  do {
    // envia um frame da fila
    socket->send_frame(f);
    response = receive_ack_nack();
    if(!response)
      return NULL;
    retries++;
  } while (response->get_dado()[0] != f->get_seq() && retries < NUM_RETRIES);

  if (response == NULL && retries == NUM_RETRIES) {
    cout << "Desisti de enviar\n";
    return NULL;
  }

  cout << "\tACK recebido:\n";
  response->imprime(HEX);
  return response;
}

// Inicia a transmissao com o servidor
int client::start_transmission() {
  cout << "\tIniciando transmissao\n";
  frame *ini = new frame(INIT, 0, vector<char>(1, 0));
  frame* enviado = new frame();
  enviado = send_frame_socket(ini);
  if (!enviado) {
    cout << "\tFalha ao iniciar a transmissao\n";
    return 0;
  }

  cout << "\tTransmissao iniciada com sucesso\n";
  return 1;
}

// Encerra a transmissao com o servidor
int client::end_transmission() {
  cout << "\tEncerrando a transmissao\n";
  frame *end = new frame(FIMT, 0, vector<char>(1, 0));
  frame* enviado = send_frame_socket(end);
  if (!enviado) {
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

  if (frames.empty()) { return 0; }

  cout << "\tstart transmission\n";
  if (!start_transmission()) { return 0; }
  cout << "\t ->>> started transmission <<< -\n";
  // Cria a fila de frames
  queue<int> janela;
  int frameCounter;
  int iniJanela = 0;
  while (iniJanela < frames.size()) {
    //manda todos os frames de uma vez só
    for(frameCounter = 0; frameCounter < TAM_JANELA; frameCounter++){
      if(iniJanela+frameCounter == frames.size())
        break;
      janela.push(frameCounter);
      
      cout << "\tEnviando frame\n";
      frames[iniJanela]->imprime(HEX);
      
      if(socket->send_frame(frames[iniJanela+frameCounter]) >= 0)
        cout << "\tFrame enviado com sucesso\n";
    }

    for (int i = 0; i < min((int)TAM_JANELA, (int)frames.size()); i++) {
      frame* res = NULL;
      int retries = 0;
      do {
        res = receive_ack_nack();
        retries++;
      } while (res == NULL && retries < NUM_RETRIES);

      if(res == NULL && retries == NUM_RETRIES){
        break;
      }
      
      if(res->get_tipo() == NACK && res->get_dado()[0] == janela.front()){
        iniJanela += res->get_dado()[0];
        janela.pop();
        break;
      }

      if(res->get_tipo() == ACK && res->get_dado()[0] == janela.front()){
        iniJanela++;
        janela.pop();
      }
      else{
        i--;
      }
    }
    //apaga a janela
    while(! janela.empty()) janela.pop();
  }

  if (!end_transmission()) { return 0; }
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
bool client::verify_ack_nack(frame *received) {
  return ((received->get_tipo() == ACK || received->get_tipo() == NACK) && received->chk_crc8());
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
    cout << "Arquivo inexistente. Operacao abortada\n";
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
    cout << "Digite o nome do arquivo(maximo de " << TAM_DADOS
         << " char):\n";
    getline(cin, fileName);
  } while (fileName.size() > TAM_DADOS);

  fileNameVector.insert(fileNameVector.begin(), fileName.begin(),
                        fileName.end());

  if (!send_message(fileNameVector, MIDIA)) {
    cout << "Falha ao enviar o arquivo\n";
    return;
  }

  cout << "Arquivo enviado com sucesso\n";
}

/**
 * @brief Send a text message through the socket
 *
 * @param message
 */
void client::send_text(string message) {

  cout << "Enviando mensagem\n";

  vector<char> data(message.begin(), message.end());
  if (!send_message(data, TEXTO))
    cout << "Limite de timout, mensagem nao foi enviada\n";

  else
    cout << "Mensagem enviada com sucesso\n";
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
    cout << "Falha ao abrir o arquivo para leitura\n";
    return vector<frame *>();
  }

  // Adiciona os dados do arquivo
  framesAux = create_frames(vectorData, DADOS);
  framesToSend.insert(framesToSend.end(), framesAux.begin(), framesAux.end());

  // Arruma a sequencia dos frames
  for (int i = 0; i < framesToSend.size(); i++)
    framesToSend[i]->set_seq(i%TAM_JANELA);

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
  if (string_has(str, CMD_HELP)) return 'h';

  if (string_has(str, CMD_EXIT)) return 'e';

  if (string_has(str, CMD_SEND)) return 's';

  return 'm';
}

void client::print_help() {
  cout << "Comandos disponiveis:\n";
  cout << "help ou -h ou h ou HELP ou Help ou H ou ajuda ou Ajuda ou AJUDA\n";
  cout << "exit ou -e ou e ou EXIT ou Exit ou E ou sair ou Sair ou SAIR\n";
  cout << "send ou -s ou s ou SEND ou Send ou S ou enviar ou Enviar ou "
          "ENVIAR\n";
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
