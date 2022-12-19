#include <iostream>
#include <bitset>

#include "frame.h"

using namespace std;

int main()
{
  
  frame f1;
  f1.set_tipo(0x10);
  f1.set_seq(0x02);
  f1.set_dado("1");

  frame f2(0x01, 0x01, "1");
  
  cout << "Frame 1\n";
  f1.imprime();
  
  cout << "\nFrame 2\n";
  f2.imprime();
}
