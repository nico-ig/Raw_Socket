#ifndef _FRAME_
#define _FRAME_

#include <bitset>

using namespace std;

// Classe que define um frame, seus campos e metodos
typedef class frame
{
  bitset<8> ini;
  bitset<6> tipo;
  bitset<4> seq;
  bitset<6> tam;
  bitset<64> dado;
  bitset<8> CRC8;
  bitset<4> padd;

} frame;

#endif
