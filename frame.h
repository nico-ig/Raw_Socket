#ifndef _FRAME_
#define _FRAME_

#include <cstring>
#include <bitset>

#include "macros.h"

// Define o polinomio usado no calculo do CRC
#define POLINOMIO 0x9B

using namespace std;

char bitSeq(string seq)
{
  int des = 0;
  char bitSeq;
  ZERA(bitSeq);

  for ( char c : seq )
    for ( int i = 0; i < sizeof(char) * 8; i++ )
      bitSeq |= ( BIT(c, i) << des++ );
 
  return bitSeq;
}

char calc_crc8(string seq)
{
  int i;
  seq = "11";
  char crc = bitSeq(seq);
  
  crc << 8;
  
  while ( crc >> 8 )
  {
    // Acha o primeiro 1
    for ( i = 0; ! BIT(crc, i); i++ );
  
    // Alinha o primeiro 1 na primeira posicao
    crc << i;
  
    // Faz a divisao com o xor entre os operandos
    crc ^= POLINOMIO;
  }

  return crc;
}

// Classe que define um frame, seus campos e metodos
#pragma pack(1)
class frame
{
  private:
    void add_ini (int cod);
    void add_tipo(int t);
    void add_seq (int s);
    void add_dado(string d);
    void add_tam(int t);
    void add_crc(string d);

  public:
    char ini :8;
    char tipo:6;
    char seq :4;
    char tam :6;
    char dado[BITPOW(6)-1];
    char crc8:8;

  // -------- Construtor -------- //
  frame(int t, int s, string d);
};

// ------------------------------ PRIVATE --------------------------------- //

void frame::add_ini(int cod) {  ini = cod; }
void frame::add_tipo(int t)  { tipo = t;   }
void frame::add_seq(int s)   {  seq = s;   }
void frame::add_tam(int t)   {  tam = t;   }

void frame::add_dado(string d) { strcpy(dado, d.c_str()); }
void frame::add_crc(string seq) { crc8 = calc_crc8(seq); }

// ------------------------------- PUBLIC --------------------------------- //
frame::frame(int t, int s, string d)
{
  add_ini(0x7E);
  add_tipo(t);
  add_seq(s);
  add_dado(d);
  add_tam(d.size());
  add_crc(d);
}

#endif
