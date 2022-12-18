#ifndef _FRAME_
#define _FRAME_

#include <cstring>
#include <bitset>

#include "macros.h"

using namespace std;

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

// ------------------------------- PUBLIC --------------------------------- //
frame::frame(int t, int s, string d)
{
  add_ini(0x7E);
  add_tipo(t);
  add_seq(s);
  add_dado(d);
  add_tam(d.size());
}

#endif
