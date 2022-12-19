#ifndef _FRAME_
#define _FRAME_

#include <bitset>
#include <cstring>
#include <iostream>

#include "macros.h"

#define UC unsigned char
#define UI unsigned int

// Define o codigo de inicio
#define INI 0x7E

// Define o polinomio usado no calculo do CRC
#define POLINOMIO 0x9B

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

    UI calc_crc8(string seq);
    UI bit_seq(string seq);

  public:
  // ------- Construtores ------ //
  frame();
  frame(int t, int s, string d);

  // ---------- Funcoes -------- //    
  UC get_ini()     { return ini;  }
  UC get_tipo()    { return tipo; }
  UC get_seq()     { return seq;  }
  UC get_tam()     { return tam;  }
  char *get_dado() { return dado; }
  UI get_crc8()    { return crc8; }

  void set_tipo(int t)    { add_tipo(t); }
  void set_seq(int s)     { add_seq(s);  }
  void set_dado(string d) { add_dado(d); }

  void imprime();
};


// ------------------------------ PRIVATE --------------------------------- //

void frame::add_ini(int cod) {  ini = cod; }
void frame::add_tipo(int t)  { tipo = t;   }
void frame::add_seq(int s)   {  seq = s;   }
void frame::add_tam(int t)   {  tam = t;   }

void frame::add_crc(string seq) { crc8 = calc_crc8(seq); }

void frame::add_dado(string d)  
{ 
  if ( d.size() > BITPOW(6)-1 ) { return; }

  strcpy(dado, d.c_str()); 
  add_tam(d.size()); 
  add_crc(d); 
}

// Transforma uma string em uma sequencias de bits do tipo unsigned int
UI frame::bit_seq(string seq)
{
  int des = 0;
  UI bit_seq;
  ZERA(bit_seq);

  for ( char c : seq )
    for ( int i = 0; i < sizeof(char) * 8; i++ )
      bit_seq |= ( BIT(c, i) << des++ );

  return bit_seq;
}

// Calcula o crc8 de uma string, para imprimir o calculo de CRC descomente
UI frame::calc_crc8(string seq)
{
  int i, des;
  int tam = seq.size() * 16;
  UI crc = bit_seq(seq);

  //cout << "Dado: ";
  //IMPRIME(crc, seq.size()*8);
  //cout << "Polinomio: ";
  //IMPRIME(POLINOMIO, 8);
  //cout << "\n";
 
  crc <<= 8;
  while ( crc >> 8 )
  {
    //IMPRIME(crc, tam);

    // Acha o primeiro 1
    for ( i = tam - 1; ! BIT(crc, i); i-- );
    
    // Calcula o deslocamento
    des = tam - i - 1;
    
    // Calcula o novo tamanho, ignora os 0 a esquerda, 
    // alinhando o primeiro 1 na primeira posicao
    tam = i + 1;
    
    //cout << "<< " << des << "\n";
    //IMPRIME(crc, tam);
    //IMPRIME(POLINOMIO << (tam - 8), tam);
    //cout << "----------------- xor\n";
    
    // Faz a divisao com o xor entre os operandos
    crc ^= (POLINOMIO << (tam - 8));
    
    //IMPRIME(crc, tam);
    //cout << "\n";
  }
 
  //cout << "crc8: "; 
  //IMPRIME(crc, 8);
  return crc;
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
