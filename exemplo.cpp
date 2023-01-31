#include <iostream>
#include <bitset>
#include <vector>
#include <cmath>

#include "crc8.h"
#include "frame.h"
#include "convolucao.h"  

#define GET_CONV(input) ((input & 0x30000) >> 16)
#define CHANGE_STATES for ( int i = 0; i < 4; i++ ) \
                        estado[i][ATUAL] = estado[i][PROX]

typedef enum { ATUAL, PROX } Estado;

using namespace std;

int main()
{
  vector<vector<int>> estado(4, vector<int> (2));

  vector<vector<int>> caminho   { { 1, 0, 0, 2},
                                  { 2, 0, 0, 1},
                                  { 0, 1, 2, 0},
                                  { 0, 2, 1, 0} };

  vector<vector<int>> distancia { { 0, 1, 1, 2 },
                                  { 1, 0, 2, 1 },
                                  { 1, 2, 0, 1 },
                                  { 2, 1, 1, 0 } };

  int input = 0xA4;
  int lg = __lg(input);
  int cntConv = ceil(lg/2.0); 

  vector<vector<int>> trelica(4, vector<int> (cntConv, -1));

  // 1o estado
  input <<= (17-lg);
  int conv = GET_CONV(input);
  estado[0][ATUAL] = distancia[0][conv];
  estado[3][ATUAL] = distancia[3][conv];
  trelica[0][0] = 0;
  trelica[3][0] = 0;

  // 2o estado
  input <<= 2;
  conv = GET_CONV(input);
  estado[0][PROX] = estado[0][ATUAL] + distancia[0][conv];
  estado[1][PROX] = estado[3][ATUAL] + distancia[1][conv];
  estado[2][PROX] = estado[3][ATUAL] + distancia[2][conv];
  estado[3][PROX] = estado[0][ATUAL] + distancia[3][conv];
  trelica[0][1] = 0;
  trelica[1][1] = 3;
  trelica[2][1] = 3;
  trelica[3][1] = 0;
  CHANGE_STATES;

  // Demais estados
  for ( int no = 2; no < cntConv; no++ )
  {
    input <<= 2;
    conv = GET_CONV(input);

    // Preenche um estado da trelica
    for ( int i = 0; i < 4; i++ )
    {
      // Preenche para cada estado j saindo de i
      for ( int j = 0; j < 4; j++ )
      {
        // Pula se n puder chegar no estado j saindo de i
        if ( ! caminho[i][j] ) { continue; }

        int chegou = trelica[j][no];

        // Se ngm tiver chego no estado j ainda
        if ( chegou == -1 ) { trelica[j][no] = i; }

        // Caso ja tenha alguem no estado j
        else 
        { 
          // Ve a distancia de quem ja ta la
          int disChegou = estado[chegou][ATUAL] + distancia[chegou][conv];

          // Ve a distancia do estado j
          int disI      = estado[i][ATUAL]      + distancia[i][conv];
          if ( disI < disChegou ) { trelica[j][no] = i; }
        }
      }
    }

    // Atualiza a distancia andada ate cada estado
    for ( int i = 0; i < 4; i++ )
    {
      int chegou = trelica[i][no];
      estado[i][PROX] = estado[chegou][ATUAL] + distancia[i][conv];
    }

    CHANGE_STATES;
  }
  
  // Acha o no onde tem que terminar
  int atual = 0;
  for ( int i = 1; i < 4; i++ )
    if ( estado[i][PROX] < estado[atual][PROX] ) { atual = i; }

  // Refaz o caminho da convolucao
  int dado = 0;
  for ( int no = cntConv - 1; no >= 0; no--)
  {
    int anterior = trelica[atual][no];
    int bit = caminho[anterior][atual] - 1;
    dado = dado | (bit << (cntConv - 1 - no));
    atual = anterior;
  }

  cout << dado << "\n";

//  gen_crc8_table();
//  gen_conv_table(5, 7);
//  string status;
//
//  frame f1;
//  f1.set_tipo(TEXTO);
//  f1.set_seq(0x01);
//  f1.set_dado("Hello World!", CONV);
//
//  cout << "Frame 1 Convolucao\n";
//  f1.imprime(HEX);
//  f1.chk_crc8() ? status = "valido" : status = "invalido";
//  cout << "Crc8 " << status << "\n";
}

