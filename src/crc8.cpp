#include <iostream>
#include <iomanip>
#include <string>

#include "../headers/crc8.h"

#define POLINOMIO 0x9B

using namespace std;

// Define o polinomio usado no calculo do CRC
uint8_t crc8_table[256];

// Gera a tabela com os valores do crc
void gen_crc8_table()
{
  // Calcula o crc para todos os valores de 1 a 255
  for ( int divident = 0; divident < 256; divident++ )
  {
    unsigned int currByte = divident; 
    
    // Calcula bit a bit
    for ( int bit = 0; bit < 8; bit++ )
    {
      currByte <<= 1;
      
      // Se o bit mais significativo for 1, faz a divisao
      if ( currByte & 0x100 ) { currByte ^= POLINOMIO; }
    }

    crc8_table[divident] = uint8_t(currByte);  
  }
}

uint8_t calc_crc8(uint8_t *msg, int tamBytes)
{
  uint8_t crc = 0;

  for ( int i = 0; i < tamBytes; i++ )
  {
    uint8_t data = msg[i] ^ crc;
    crc = crc8_table[data];
  }
  
  return crc;
}
