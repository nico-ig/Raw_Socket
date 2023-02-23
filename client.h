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

#include <arpa/inet.h>

// include local
#include "conexao.h"
#include "crc8.h"
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
  conexao *local;
  conexao *target;

  // ---------- Funcoes -------- //
  int send_frames(vector<frame *> frames);
  int send_message(string data, int type);
  void send_file();
  void send_text(string message);
  bool string_has(string str, vector<string> strList);
  char string_cmd(string str);
  void print_help();
  bool verify_ack(frame *received, frame *sent);
  vector<frame *> create_frames(string data, int type);
  string add_escapes(string data);

public:
  // --------- Dados ---------- //
  string userInput;

  // ------- Construtores ------ //
  client(conexao *local, conexao *target);

  // ---------- Funcoes -------- //
  void run();
};

// ------------------------------ PRIVATE --------------------------------- //

/**
 * @brief Send a list of frames through the socket
 *
 * @param f
 * @return int
 */
int client::send_frames(vector<frame *> frames) {

  vector<int> timeouts = {1, 2, 4, 8, 16, 32};
  bool ack = false;

  // Envia um frame por vez
  for (size_t i = 0; i < frames.size(); i++) {

    cout << "\tEnviando frame\n";
    frames[i]->imprime(DEC);

    int timeout = 0;
    // Fica tentando enviar o frame até receber o ack
    while (!ack && (timeout < timeouts.size())) {

      target->send_frame(frames[i]);

      /*wait for ack*/
      frame *f = local->receive_frame();
      cout << "\tRecebendo ack\n";
      // f->imprime(DEC);
      ack = verify_ack(f, frames[i]);
      cout << "\tAck verificado: " << ack << "\n";

      if (!ack) {
        cout << "Falha no envio, timeout de: " << timeouts[timeout] << "seg\n";
        sleep(timeouts[timeout]);
        timeout++;
      } else
        timeout = 0;
    }

    // Se tentar 10 vezes e nao conseguir, desiste de enviar
    if (timeout == timeouts.size())
      return 0;

    cout << "\tFrame enviado com sucesso\n";
  }

  cout << "Terminou de enviar todos os frames\n";
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

  // cout << "Verificando ack\n";
  // cout << "ver-Tipo: " << static_cast<int>(received->get_tipo()) << " "
  //      << static_cast<int>(ACK) << " = "
  //      << (received->get_tipo() == ACK) << "\n";
  // cout << "ver-Seq: " << received->get_seq() << " " << sent->get_seq() << " = " << (received->get_seq() == sent->get_seq()) << "\n";
  // cout << "ver-Dado: " << received->get_dado() << " " << sent->get_dado() << " = " << (!strcmp(received->get_dado(),sent->get_dado())) << "\n";
  // cout << "ver-CRC: " << received->chk_crc8() << "\n";

  return (
      received->get_tipo() == ACK && received->get_seq() == sent->get_seq() &&
      !strcmp(received->get_dado(), sent->get_dado()) && received->chk_crc8());
}

/**
 * @brief Send data through the socket
 *
 * @param data: data to be sent
 * @return int
 */
int client::send_message(string data, int type) {

  int messageSize = data.size();
  vector<frame *> frames;
  frames = create_frames(data, type);

  return send_frames(frames);
}

/**
 * @brief Send a file through the socket
 *
 */
void client::send_file() {
  cout << "Enviando arquivo\n";
  string fileName;
  cout << "Digite o nome do arquivo:\n";
  getline(cin, fileName);

  if (!send_message(fileName, MIDIA))
    cout << "Limite de timout, arquivo nao foi enviado\n";

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

  if (!send_message(message, TEXTO))
    cout << "Limite de timout, mensagem nao foi enviada\n";

  else
    cout << "Mensagem enviada com sucesso\n";
}

/**
 * @brief Creates a vector with all the frames to be sent
 *
 * @param message
 */

vector<frame *> client::create_frames(string data, int type) {
  vector<frame *> frames;
  int i = 0;
  string message = add_escapes(data);

  while (i < message.size()) {
    frame *f = new frame();
    f->set_tipo(type);
    f->set_seq(i);
    f->set_dado(message.substr(i, 63));
    frames.push_back(f);
    i += 63;
  }

  return frames;
}

/*-- Não são os escapes certos, só queria deixar pronto essa parte para
   botar os escapes certos--*/
/**
 * @brief Add escape characters to the data to be sent
 *
 * @param data
 * @return string
 */
string client::add_escapes(string data) {

  string message = "";
  for (size_t i = 0; i < data.size(); i++) {

    switch (data[i]) {

    case 0x7E:
      message += 0x7D;
      message += 0x5E;
      break;

    case 0x7D:
      message += 0x7D;
      message += 0x5D;
      break;

    default:
      message += data[i];
      break;
    }
  }

  return message;
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

/**
 * @brief Construct a new client::client object
 *
 * @param localConnection local connection to listen for messages
 * @param targetConnection  target connection to send frames
 */
client::client(conexao *localConnection, conexao *targetConnection) {
  local = localConnection;
  target = targetConnection;
}

// lock the thread with mutex
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
