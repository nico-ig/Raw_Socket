#include <iostream>
#include <vector>

// include local
#include "../headers/conexao.h"
#include "../headers/crc8.h"
#include "../headers/frame.h"
#include "../headers/macros.h"
#include "../headers/server.h"
#include "../headers/client.h"

using namespace std;

#define HEX "%02x"

typedef enum { CLIENT, SERVER } STATUS_E;

int get_status( char *argv )
{
  if ( !strcmp( argv, "client") ) { return CLIENT; } 
  if ( !strcmp( argv, "server") ) { return SERVER; } 
  else                           { return -1; }
}

int main(int argc, char *argv[]) {

  gen_crc8_table();
  char* device = argv[2];
  cout << "Device: " << device << endl;
  conexao socket(device);

  int status = get_status(argv[1]);
  switch ( status )
  {
    case CLIENT:
    {
      client cliente(&socket);
      cliente.run();
    }
      break;
    
    case SERVER:
    {
      server servidor(&socket);
      servidor.run();
    }
      break;

    default:
      cout << "Comando invalido\n";
      break;
  }   



//  thread clientSend(&client::run, &cliente);

//  server servidor(&local, &target);
//  thread serverReceive(&server::run, &servidor);

//  int receive = 0;
//  while (true) {
//    receive++;
//  }

// serverReceive.join();
//  clientSend.join();

  return 0;
}

