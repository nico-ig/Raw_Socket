// Receive all the data
#ifndef _CLIENT_
#define _CLIENT_

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
#include "conexao.h"
#include "crc8.h"
#include "frame.h"
#include <mutex>
std::mutex mtx;

vector<string> CMD_HELP = {"H",  "help",  "-h",    "HELP", "Help",
                           "-H", "ajuda", "Ajuda", "AJUDA"};
vector<string> CMD_EXIT = {"exit", "-e",   "EXIT", "Exit",
                           "-E",   "sair", "Sair", "SAIR"};
vector<string> CMD_SEND = {"send", "-s",     "SEND",   "Send",
                           "-S",   "enviar", "Enviar", "ENVIAR"};

using namespace std;

class client {

private:
  int soquete;
  void send_file();
  void send_text(string message);

  int send_frames(vector<frame *> frames);
  int send_message(string data, int type);

  vector<frame *> framesMidia;
  conexao *local;
  conexao *target;

public:
  client(conexao *local, conexao *target);
  string userInput;
  // ~server();
  void run();
};

bool string_has(string str, vector<string> strList) {
  for (int i = 0; i < strList.size(); i++) {
    if (str.find(strList[i]) != string::npos) {
      return true;
    }
  }
  return false;
}

char *string_cmd(string str) {
  if (string_has(str, CMD_HELP))
    return "h";
  if (string_has(str, CMD_EXIT))
    return "e";
  if (string_has(str, CMD_SEND))
    return "s";
  return "m";
}

/**
 * @brief Send a list of frames through the socket
 *
 * @param f
 * @return int
 */
int client::send_frames(vector<frame *> frames) {
  vector<int> timeouts = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
  int timeout = 0;
  bool ack = false;
  for (size_t i = 0; i < frames.size(); i++) {
    while (!ack || timeout < 10) {
      /* code */
      /*send frame*/
      target->send_frame(frames[i]);
      /*wait for ack*/
      frame *f = local->receive_frame();
      ack = local->verify_ack(f, frames[i]);

      if (!ack)
        timeout++;
      else
        timeout = 0;
      sleep(timeouts[timeout]);
    }
  }
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
  frames = target->create_frames(data);
  return send_frames(frames);
};

/**
 * @brief Send a file through the socket
 *
 */
void client::send_file() {
  cout << "Enviando arquivo\n";
  string fileName;
  cout << "Digite o nome do arquivo:\n";
  getline(cin, fileName);
  send_message(fileName, MIDIA);
}

/**
 * @brief Send a text message through the socket
 *
 * @param message
 */
void client::send_text(string message) {
  cout << "Enviando mensagem\n";
  send_message(message, TEXTO);
};

void print_help() {
  cout << "Comandos disponiveis:\n";
  cout << "help ou -h ou h ou HELP ou Help ou H ou ajuda ou Ajuda ou AJUDA\n";
  cout << "exit ou -e ou e ou EXIT ou Exit ou E ou sair ou Sair ou SAIR\n";
  cout
      << "send ou -s ou s ou SEND ou Send ou S ou enviar ou Enviar ou ENVIAR\n";
  cout << "para enviar uma mensagem, digite a mensagem e pressione enter\n";
};
void client::run() {
  // lock the thread with mutex
  int i;
  while (true) {
    cout << " Digite um comando ou mensagem:\n";
    getline(cin, userInput);
    char *userInputCMD = string_cmd(userInput);
    switch (*userInputCMD) {
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
      cout << "Enviando mensagem\n";
      send_text(userInput);
      break;
    default:
      cout << "Comando invalido\n";
      break;
    }
  }
}

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

#endif
