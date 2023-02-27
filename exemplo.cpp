#include <iostream>
#include <bitset>
#include <vector>
#include <cmath>
#include <time.h>

#include "crc8.h"
#include "frame.h"
#include "convolucao.h"  

using namespace std;

// Retorna um inteiro aleatorio entre lo e hi
int alet(int lo, int hi)
{
  return lo + (rand() % (hi - lo + 1));
}

int main()
{
  srand(2);

  gen_crc8_table();
  gen_conv_table(5, 7);
  string dado = "Hello World!";

  printf("\n============================================================\n\n");

  printf("Dado       : ");
  for ( int i = 0; i < dado.size(); i++ )
    printf("0x%X ", dado[i]); 
  printf("\n");

  printf("\n============================================================\n\n");

  printf("Dado conv  : ");
  uint16_t *dado_conv = gen_conv((uint8_t *)dado.c_str(), dado.size());
  uint8_t dado_tmp[dado.size()*2];

  // Copia o dado
  for ( int i = 0; i < dado.size(); i++)
  {
    dado_tmp[i*2] = (dado_conv[i] >> 8) & 0xFF;
    dado_tmp[i*2+1] = dado_conv[i] & 0xFF;
  }

  for ( int i = 0; i < dado.size() * 2; i++ )
    printf("0x%X ", dado_tmp[i]); 
  printf("\n");

  printf("\n============================================================\n\n");

  // Muda alguns bytes do dado
  int num_changes = 1;
  for ( int changed = 0, i = 0; changed < num_changes && i < dado.size(); i++ )
  {
    int err_bit = alet(0, 1);
    if ( err_bit )
    {
      changed++;
      dado_tmp[i] = 0x1;
    }
  }
  
  printf("Dado errado: ");
  for ( int i = 0; i < dado.size() * 2; i++ )
    printf("0x%X ", dado_tmp[i]); 
  printf("\n");


  printf("\n============================================================\n\n");

  char *dado_deconv = deconv(dado_tmp, dado.size() * 2);

  printf("\n============================================================\n\n");

  printf("Dado deconv: ", dado_deconv);
  for ( int i = 0; i < dado.size(); i++ )
    printf("0x%X ", dado_deconv[i]); 
  printf("\n");
  printf("Dado       : %s\n", dado_deconv);

  printf("\n============================================================\n\n");

  printf("\nFrame 1\n");

  printf("\n============================================================\n\n");

  frame f1;
  f1.set_tipo(TEXTO);
  f1.set_seq(0x01);
  f1.set_dado("Hello World!", CONV);
  f1.deconv_dado();

  printf("\n============================================================\n\n");

  string status;
  f1.imprime(HEX);
  f1.chk_crc8() ? status = "valido" : status = "invalido";
  printf("Crc8 %s\n", status.c_str());
}



