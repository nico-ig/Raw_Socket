#ifndef _FRAME_
#define _FRAME_

#include <bitset>
#include <cstring>
#include <iostream>

#include "crc8.h"
#include "macros.h"

// Define o codigo de inicio
#define INI 0x7E

using namespace std;

// Enum para os tipos de frame
typedef enum 
{
  TEXTO = 0x01,
  MIDIA = 0x10,
  ACK   = 0x0A,
  NACK  = 0x00,
  ERRO  = 0x1E,
  INIT  = 0x1D,
  FIMT  = 0x0F,
  DADOS = 0x0D,

} Tipo;

// Classe que define um frame, seus campos e metodos
#pragma pack(1)
class frame
{
  private:
  // --------- Dados ---------- //
    UC ini :8;
    UC tipo:6;
    UC seq :4;
    UC tam :6;
    char dado[BITPOW(6)-1];
    UI crc8:8;

  // ---------- Funcoes -------- //    
    void add_ini (int cod);
    void add_tipo(int t);
    void add_seq (int s);
    void add_dado(string d);
    void add_tam(int t);
    void add_crc(string d);

  public:
  // ------- Construtores ------ //
  frame();
  frame(int t, int s, string d);

  // ---------- Funcoes -------- //    
  UC get_ini()     { return ini;  }
  UC get_tipo()    { return tipo; }
  UC get_seq()     { return seq;  }
  UC get_tam()     { return tam;  }
  UI get_crc8()    { return crc8; }
  char *get_dado() { return dado; }

  void set_tipo(int t)    { add_tipo(t); }
  void set_seq(int s)     { add_seq(s);  }
  void set_dado(string d) { add_dado(d); }

  void imprime();
  int chk_crc8() { return ! calc_crc8(dado, crc8); }

};


// ------------------------------ PRIVATE --------------------------------- //

void frame::add_ini(int cod) {  ini = cod; }
void frame::add_tipo(int t)  { tipo = t;   }
void frame::add_seq(int s)   {  seq = s;   }
void frame::add_tam(int t)   {  tam = t;   }

void frame::add_crc(string seq) { crc8 = calc_crc8(seq, 0); }

void frame::add_dado(string d)  
{ 
  if ( d.size() > BITPOW(6)-1 ) { return; }

  strcpy(dado, d.c_str()); 
  add_tam(d.size()); 
  add_crc(d); 
}

// ------------------------------- PUBLIC --------------------------------- //
frame::frame() { add_ini(INI); }

frame::frame(int t, int s, string d)
{
  add_ini(INI);
  add_tipo(t);
  add_seq(s);
  add_dado(d);
}

void frame::imprime()
{
  cout << "Ini:  ";
  IMPRIME(INI, 8);

  cout << "Tipo: ";
  IMPRIME(tipo, 6);

  cout << "Seq:  ";
  IMPRIME(seq, 4);

  cout << "Tam:  ";
  IMPRIME(tam, 6);

  cout << "Dado: ";
  IMPRIME(bit_seq(dado), tam);

  cout << "Crc8: ";
  IMPRIME(crc8, 8);
}

#endif
