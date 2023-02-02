#include <iostream>
#include <bitset>
#include <vector>
#include <cmath>

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
  f1.set_dado("Hello World!", CONV);
  f1.deconv_dado();

  cout << "Frame 1 Convolucao\n";
  f1.imprime(DEC);
  f1.chk_crc8() ? status = "valido" : status = "invalido";
  cout << "Crc8 " << status << "\n";
}

