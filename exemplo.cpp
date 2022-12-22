#include <iostream>
#include <bitset>

#include "frame.h"

using namespace std;

int main()
{
  string status;

  frame f1;
  f1.set_tipo(0x10);
  f1.set_seq(0x02);
  f1.set_dado("1");

  frame f2(0x01, 0x01, "1");
  
  cout << "Frame 1\n";
  f1.imprime();
  
  f1.chk_crc8() ? status = "valido" : status = "invalido";
  cout << "Crc do frame 1 " << status << "\n";

  cout << "\nFrame 2\n";
  f2.imprime();

  f2.chk_crc8() ? status = "valido" : status = "invalido";
  cout << "Crc do frame 2 " << status << "\n";
}
