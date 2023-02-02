#include <iostream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <math.h>

#include "convolucao.h"

using namespace std;

// Gera a convolucao de um bit 
#define NEXT_STATE ( ((__builtin_popcount(currState & PG.first ) & 1) << 1) \
                    | (__builtin_popcount(currState & PG.second) & 1)       )

// Pega uma convolucao de um input
#define GET_CONV(input) ((input & 0x300) >> 8)

// Muda os estados de cada no
#define CHANGE_STATES for ( int i = 0; i < 4; i++ ) \
                        estado[i][ATUAL] = estado[i][PROX]

// Enum para acessar os estados da trelica
typedef enum { ATUAL, PROX } Estado;

/* ------------------------------------------- Global ------------------- */

// Define o polinomio gerador da convolucao 
pair<uint8_t, uint8_t> PG;

// Tabela de convolucao para um byte
uint16_t conv_table[4][256]; 

// Variavel global para armazenar a o estado inicial para uma convolucao
uint8_t INITIAL_STATE = 0;

// Matriz com a quantidade de mudancas em cada no, atual e prox 
vector<vector<int>> estado(4, vector<int> (2));

// Matriz com o melhor caminho para cada no 
vector<vector<int>> trelica;

// Caminho possiveis de um no para outro de acordo com a entrada
vector<vector<int>> caminho   { { 1, 0, 0, 2},
                                { 2, 0, 0, 1},
                                { 0, 1, 2, 0},
                                { 0, 2, 1, 0} };

// Distancia entre dois pares de bits
vector<vector<int>> distancia { { 0, 1, 1, 2 },
                                { 1, 0, 2, 1 },
                                { 1, 2, 0, 1 },
                                { 2, 1, 1, 0 } };


/* -------------------------- Deconvolucionar -------------------------------- */

// Gera a tabela de convolucao (1/2) para os polinomio p1 e p2. Por mais q o 
// parametro seja um inteiro, os polinomios devem ter 3 bits 
void gen_conv_table(int p1, int p2)
{
  // Adiciona os polinomios ao par
  PG.first = p1;
  PG.second = p2;

  // Gera uma tabela de convolucao para cada estado inicial possivel
  for ( INITIAL_STATE = 0; INITIAL_STATE < 4; INITIAL_STATE++ )
  {

    // Faz a convergencia para todos os bytes
    for ( uint16_t byte = 0; byte < 256; byte++ )
    {
      uint16_t conv = 0; 
      uint8_t input = 0;
      uint8_t currState = 0;
      uint16_t currByte = byte;
      uint8_t currConv = INITIAL_STATE;

      // Faz a convolucao parcial bit a bit
      for ( uint8_t bit = 0; bit < 8; bit++ )
      {
        // Retira o bit mais significativo
        currByte <<= 1;

        // Pega o bit que foi retirado 
        (currByte & 0x100) ? input = 1 : input = 0;

        // Determina o estado atual da maquina
        currState = ( (input << 2) | currConv );

        // Atualiza a convolucao
        conv = (conv << 2) | NEXT_STATE;
        currConv = NEXT_STATE;
      }

      conv_table[INITIAL_STATE][byte] = conv;
    }
  }

  INITIAL_STATE = 0;
}

// Retorna a convolucao de uma determinada string
uint16_t *gen_conv(uint8_t *msg, int tamBytes)
{
  uint16_t *msgConv;

  if ( !(msgConv = (uint16_t *)malloc(sizeof(uint16_t)*tamBytes)) ) {exit(0);}

  // Calcula a convolucao para cada byte
  for ( int i = 0; i < tamBytes; i++ )
  {
    uint8_t byte = msg[i];
    uint16_t conv = conv_table[INITIAL_STATE][byte]; 
    msgConv[i] = conv;

    INITIAL_STATE = conv & 3;
  }

  INITIAL_STATE = 0;
  return msgConv;
}

/* -------------------------- Deconvolucionar ------------------------------ */

// Preenche o estado da trelica para uma convolucao em um determinado no
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
        int disChegou = estado[chegou][ATUAL];

        // Ve a distancia do estado j
        int disI = estado[i][ATUAL];
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

// Inicializa o primeiro no diferente de zero da trelica
void init_first_no(int firstNo, int conv)
{
  estado[0][ATUAL] = distancia[0][conv];
  estado[3][ATUAL] = distancia[3][conv];
  trelica[0][firstNo] = 0;
  trelica[3][firstNo] = 0;
}

// Inicializa o segundo no diferente de zero da trelica
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

// Ve os caminhos da convolucao de um byte
void conv_byte(int input, int ini)
{
  for ( int no = ini; no < 4 + ini; no++ )
  {
    int conv = GET_CONV(input << 2);
    input <<= 2;
    fill_state_byte(no, conv);
  }
}

// Ve os caminhos possiveis do primeiro byte diferente de zero
void conv_byte_init(int input, int firstNo)
{
  for ( int no = firstNo; no < floor(firstNo/4) + 4; no++ )
  {
    int conv = GET_CONV(input << 2);
    input <<= 2;
    fill_state_byte(no, conv);
  }
}

// Inicializa a trelica
void init_trelica(uint8_t input, int tamBytes)
{
  trelica.resize(4, vector<int>(tamBytes * 4, -1));

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
  
  firstNo += 2;
  conv_byte_init(input, firstNo);
}

// Acha o estado em que a trelica termina
int find_last_state()
{
  // Acha o no onde tem que terminar
  int atual = 0;
  for ( int i = 1; i < 4; i++ )
    if ( estado[i][PROX] < estado[atual][PROX] ) { atual = i; }

  return atual;
}

// Acha o caminho percorrido
uint8_t backtrack_conv(int *ini, int *beginByte)
{
  // Refaz o caminho da convolucao
  uint8_t dado = 0;
  int atual = *ini;
  for ( int no = *beginByte, des = 7; no > *beginByte - 8; no--, des++)
  {
    int anterior = trelica[atual][no];
    int bit = caminho[anterior][atual] - 1;
    dado = dado | (bit << (abs(7 - des)));
    atual = anterior;
  }

  *ini = atual;
  *beginByte -= 8;
  return dado;
}

// Retorna a sequencia original de uma entrada
char *deconv(uint8_t *msg, int tamBytes)
{ 
  // Aloca um vetor para a mensagem original
  int tamOrigMsg = tamBytes/2;
  uint8_t msgOrig[tamOrigMsg + 1];

  // Monta a trelica
  init_trelica(msg[0], tamBytes);

  for ( int byte = 1; byte < tamBytes; byte++ )
    conv_byte(msg[byte], byte*4);
  
  // Imprime a trelica
//  cout << "   ";
//  for ( int i = 0; i < 8; i++ )
//    printf("%2d ", i);
//  cout << "\n";
//
//  for ( int j = 0; j < 4; j++ )
//  {
//    printf("%2d ", j);
//    for ( int i = 0; i < 8; i++ )
//      printf("%2d ", trelica[j][i]);
//    cout << "\n";
//  }

  // Corrige os erros e deconvoluciona a msg
  int ini = find_last_state();
  int beginByte = tamBytes * 4 - 1;
  for ( int i = tamOrigMsg - 1; i >= 0; i-- )
    msgOrig[i] = backtrack_conv(&ini, &beginByte); 

  // Copia a msg deconvolucionada para o vetor a ser retornado
  char *dado;
  if ( ! (dado = (char *) malloc(sizeof(char) * tamOrigMsg + 1)) ) { exit(0); }
  memmove(dado, msgOrig, tamOrigMsg);
  dado[tamOrigMsg] = 0;

  return dado;
}


