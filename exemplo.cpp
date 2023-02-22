#include <iostream>
#include <bitset>

#include "crc8.h"
#include "frame.h"

using namespace std;

int main()
{
  gen_crc8_table();
  string status;

  frame f1;
  f1.set_tipo(TEXTO);
  f1.set_seq(0x01);
  f1.set_dado("0");

  frame f2(MIDIA, 0x02, "1");
  frame f3(ACK, 0x03, "Hello World!");
  
  cout << "Frame 1\n";
  f1.imprime(BIN);
  f1.chk_crc8() ? status = "valido" : status = "invalido";
  cout << "Crc8 " << status << "\n";

  cout << "\nFrame 2\n";
  f2.imprime(HEX);
  f2.chk_crc8() ? status = "valido" : status = "invalido";
  cout << "Crc8 " << status << "\n";

  cout << "\nFrame 3\n";
  f3.imprime(DEC);
  f3.chk_crc8() ? status = "valido" : status = "invalido";
  cout << "Crc8 " << status << "\n";

}
