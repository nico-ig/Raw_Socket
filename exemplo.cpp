#include <iostream>
#include <bitset>
#include <vector>

#include "crc8.h"
#include "frame.h"
#include "convolucao.h"  

using namespace std;

int main()
{
  gen_crc8_table();
  gen_conv_table(5, 7);
  string status;

  frame f1;
  f1.set_tipo(TEXTO);
  f1.set_seq(0x01);
  
  string dado = "\a";
  vector<uint16_t> conv = set_conv(dado);

  string dadoConv;
  for ( uint16_t byteConv : conv )
  {
    printf("0x%X\n", byteConv);
    dadoConv.push_back(byteConv & 0xff);
    dadoConv.push_back((byteConv >> 8) & 0xff);
  }

  f1.set_dado(dadoConv);

  cout << "Frame 1 Convolucao\n";
  f1.imprime(HEX);
  f1.chk_crc8() ? status = "valido" : status = "invalido";
  cout << "Crc8 " << status << "\n";
}

