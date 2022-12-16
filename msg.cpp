#include <iostream>
#include <bitset>

#include "frame.h"

using namespace std;

// Macro que fica em 1 ou 0 de acordo com o valor do bit avaliado
#define BIT(value, des) ( (value & (1 << des)) ? 1 : 0 )

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

  frame f;
  cout << sizeof(f);
}
