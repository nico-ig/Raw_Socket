#include <iostream>
#include <iomanip>
#include <vector>

#include "convolucao.h"

using namespace std;

// Gera a convolucao de um bit 
#define NEXT_STATE ( ((__builtin_popcount(currState & PG.first ) & 1) << 1) \
                    | (__builtin_popcount(currState & PG.second) & 1)       )

// Define o polinomio gerador da convolucao 
pair<uint8_t, uint8_t> PG;

// Tabela de convolucao para um byte
uint16_t conv_table[4][256]; 

// Variavel global para armazenar a o estado inicial para uma convolucao
uint8_t INITIAL_STATE = 0;

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

      // Faz a convergencia parcial bit a bit
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

  if ( ! (msgConv = (uint16_t *) malloc(sizeof(uint16_t) * tamBytes)) ) { exit(0); }

  // Calcula a convergencia para cada byte
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


