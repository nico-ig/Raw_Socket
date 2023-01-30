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

typedef enum { BIN, HEX, DEC } Base;

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
  CONV  = 0x11 

} Tipo;

// Classe que define um frame, seus campos e metodos
#pragma pack(1)
class frame
{
  private:
  // --------- Dados ---------- //
    uint8_t    ini;
    uint8_t tipo:6;
    uint8_t seq :4;
    uint8_t  tam:6;
    char  dado[63];
    uint8_t   crc8;

  // ---------- Funcoes -------- //    
    void  add_ini(int cod);
    void add_tipo(int t);
    void  add_seq(int s);
    void add_dado(string d);
    void  add_tam(int t);
    void  add_crc(uint8_t *d);
    
    void imprime_bin();
    void imprime_hex();
    void imprime_dec();
  
  public:
  // ------- Construtores ------ //
  frame();
  frame(int t, int s, string d);

  // ---------- Funcoes -------- //    
  uint8_t   get_ini() { return ini;  }
  uint8_t  get_tipo() { return tipo; }
  uint8_t   get_seq() { return seq;  }
  uint8_t   get_tam() { return tam;  }
  uint8_t  get_crc8() { return crc8; }
     char *get_dado() { return dado; }

  void     set_seq(int s) { add_seq(s);  }
  void    set_tipo(int t) { add_tipo(t); }
  void set_dado(string d) { add_dado(d); }

  int chk_crc8() { return (calc_crc8((uint8_t *)dado, tam) == crc8); }

  void imprime(int base);

};


// ------------------------------ PRIVATE --------------------------------- //

void frame::add_ini(int cod) { ini = cod;  }
void frame::add_tipo(int t)  { tipo = t;   }
void frame::add_seq(int s)   {  seq = s;   }
void frame::add_tam(int t)   {  tam = t;   }

void frame::add_crc(uint8_t *d) { crc8 = calc_crc8(d, tam); }

void frame::add_dado(string d)  
{ 
  if ( d.size() > 63 ) { return; }

  add_tam(d.size()); 
  strcpy(dado, d.c_str()); 
  add_crc((uint8_t *)dado); 
}

void frame::imprime_bin()
{
  cout << " Ini: "; IMPRIME(ini, 8);
  cout << "Tipo: "; IMPRIME(tipo, 6);
  cout << " Seq: "; IMPRIME(seq, 4);
  cout << " Tam: "; IMPRIME(tam, 8);
  cout << "Dado: " ; 
  for ( int i = 0; i < tam; i++ )
  {
    unsigned int c = dado[i];
    for ( int j = 0; j < 8; j++ ) 
    {
      c <<= 1;
      if ( c & 0x100 ) { cout << "1"; }
      else             { cout << "0"; }
    }
    cout << " ";
  }
  cout << "\n";
  cout << "Crc8: "; IMPRIME(crc8, 8);
}

void frame::imprime_hex()
{
  printf(" Ini: 0x%X\n",  ini);
  printf("Tipo: 0x%X\n", tipo);
  printf(" Seq: 0x%X\n",  seq);
  printf(" Tam: 0x%X\n",  tam);
  printf("Dado:");
  for ( int i = 0; i < tam; i++ ) { printf(" 0x%X", dado[i]); }
  printf("\n");
  printf("Crc8: 0x%X\n", crc8);
}

void frame::imprime_dec()
{
  printf(" Ini: %d\n",  ini);
  printf("Tipo: %d\n", tipo);
  printf(" Seq: %d\n",  seq);
  printf(" Tam: %d\n",  tam);
  printf("Dado: %s\n", dado);
  printf("Crc8: %d\n", crc8);
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

void frame::imprime(int base)
{
  switch (base)
  {
    case BIN: imprime_bin(); break;
    case HEX: imprime_hex(); break;
    default:  imprime_dec(); break;
  }
}

#endif
