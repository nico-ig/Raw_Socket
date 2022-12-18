#include <iostream>
#include <bitset>

#include "frame.h"

using namespace std;

int main()
{
//  string str = "Hello World!";
// 
//  int des = 0;
//  int tam = str.size();
//  bitset<100> frame;
//
//  // Converte uma string para um bitset
//  for ( char c : str )
//    for ( int i = 0; i < sizeof(char) * 8; i++ )
//      frame |= ( BIT(c, i) << des++ );
//
//  cout << frame << "\n";

  frame f(0x01, 0x01, "oiii");
  cout << sizeof(frame) << "\n";
}
