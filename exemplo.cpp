#include <iostream>
#include <bitset>
#include <vector>
#include <cmath>

#include "crc8.h"
#include "frame.h"
#include "convolucao.h"  

#define GET_CONV(input) ((input & 0x300) >> 8)
#define CHANGE_STATES for ( int i = 0; i < 4; i++ ) \
                        estado[i][ATUAL] = estado[i][PROX]

typedef enum { ATUAL, PROX } Estado;

using namespace std;

vector<vector<int>> estado(4, vector<int> (2));
vector<vector<int>> trelica(4, vector<int> (32, -1));

vector<vector<int>> caminho   { { 1, 0, 0, 2},
                                { 2, 0, 0, 1},
                                { 0, 1, 2, 0},
                                { 0, 2, 1, 0} };

vector<vector<int>> distancia { { 0, 1, 1, 2 },
                                { 1, 0, 2, 1 },
                                { 1, 2, 0, 1 },
                                { 2, 1, 1, 0 } };

void fill_state_byte(int no, int conv)
{
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

void init_first_no(int firstNo, int conv)
{
  estado[0][ATUAL] = distancia[0][conv];
  estado[3][ATUAL] = distancia[3][conv];
  trelica[0][firstNo] = 0;
  trelica[3][firstNo] = 0;
}

void init_second_no(int firstNo, int conv)
{
  estado[0][PROX] = estado[0][ATUAL] + distancia[0][conv];
  estado[1][PROX] = estado[3][ATUAL] + distancia[1][conv];
  estado[2][PROX] = estado[3][ATUAL] + distancia[2][conv];
  estado[3][PROX] = estado[0][ATUAL] + distancia[3][conv];
  trelica[0][firstNo+1] = 0;
  trelica[1][firstNo+1] = 3;
  trelica[2][firstNo+1] = 3;
  trelica[3][firstNo+1] = 0;
  CHANGE_STATES;
}

void conv_byte(int input, int ini)
{
  for ( int no = ini; no < 4 + ini; no++ )
  {
    int conv = GET_CONV(input << 2);
    input <<= 2;
    fill_state_byte(no, conv);
  }
}

void conv_byte_init(int input, int firstNo)
{
  for ( int no = firstNo; no < 2 + firstNo; no++ )
  {
    int conv = GET_CONV(input << 2);
    input <<= 2;
    fill_state_byte(no, conv);
  }
}

void init_trelica(uint8_t input)
{
  // 1o estado
  int lg = __lg(input);
  int firstNo = (7-lg)/2;

  // Caminho de 0 atr a primeira conv diferente de 0
  for ( int i = 0; i < firstNo; i++ )
    trelica[0][i] = 0;

  // 1o no diferente de 0
  int conv = GET_CONV(input << 9-lg);
  input <<= 9-lg;
  init_first_no(firstNo, conv);

  // 2o no diferente de 0
  conv = GET_CONV(input << 2);
  input <<= 2;
  init_second_no(firstNo, conv);

  conv_byte_init(input, firstNo + 2);
}

int find_last_state()
{
  // Acha o no onde tem que terminar
  int atual = 0;
  for ( int i = 1; i < 4; i++ )
    if ( estado[i][PROX] < estado[atual][PROX] ) { atual = i; }

  return atual;
}

uint8_t backtrack_conv(int *ini, int *beginByte)
{
  // Refaz o caminho da convolucao
  uint8_t dado = 0;
  int atual = *ini;
  for ( int no = *beginByte; no > *beginByte - 8; no--)
  {
    int anterior = trelica[atual][no];
    int bit = caminho[anterior][atual] - 1;
    dado = (dado << 1) | bit;
    atual = anterior;
  }

  *ini = atual;
  *beginByte -= 8;
  return dado;
}

int main()
{
  uint8_t msg[63]= { 0xA4, 0xA4 };
  int tamBytes = 2;

// ============================================================ //
  int tamOrigMsg = ceil(tamBytes/2);

  uint8_t msgOrig[tamOrigMsg];

  init_trelica(msg[0]);

  for ( int byte = 1; byte < tamBytes; byte++ )
    conv_byte(msg[byte], byte*4);

  int ini = find_last_state();
  int beginByte = tamBytes * 4 - 1;
  for ( int i = 0; i < tamOrigMsg; i++ )
    msgOrig[i] = backtrack_conv(&ini, &beginByte); 

  char dado[tamOrigMsg + 1];
  memmove(dado, msgOrig, tamOrigMsg);
  dado[tamOrigMsg] = 0;
  //printf("0x%X\n", dado[0] & 0xFF );

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

