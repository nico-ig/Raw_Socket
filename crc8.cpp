#include <iostream>
#include <iomanip>
#include <string>

#include "crc8.h"
#include "macros.h"

#define POLINOMIO 0x9B

using namespace std;

// Define o polinomio usado no calculo do CRC
UI crc8_table[256];

// Imprime uma linha da tabela crc
void print_line(int ini, int fim)
{
  for ( int i = ini; i < fim; i++ )
  {
    cout << setfill(' ') << setw(3) << right << dec << i << "=0x";
    cout << setfill('0') << setw(2) << right << hex << crc8_table[i] << " ";
  }

  cout << "\n";
}

// Imprime a tabela para do crc para um determinado polinomio
void print_crc8_table()
{
  for ( int i = 0; i < 256; i+=8 )
    print_line(i, i+8);
}

// Transforma uma string em uma sequencias de bits do tipo unsigned int
UI bit_seq(string seq)
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
UI calc_crc8_table(string seq)
{
  int i, des;
  UI crc = bit_seq(seq);
  
  // size retorna o tamanho em bytes, +7 por conta do shift
  int tam = seq.size() * 8 + 7;

  //cout << "Dado: ";
  //IMPRIME(crc, seq.size()*8);
  //cout << "Polinomio: ";
  //IMPRIME(POLINOMIO, 8);
  //cout << "\n";
 
  crc <<= 7;

  while ( crc >> 7 )
  {
    //IMPRIME(crc, tam);

    // Acha o primeiro 1
    for ( i = tam - 1; ! BIT(crc, i); i-- );
    
    // Calcula o deslocamento, o 1 mais significativo eh "jogado fora"
    des = tam - i;
    tam = i;
    
    //cout << "<< " << des << "\n";
    //IMPRIME(crc, tam);
    //IMPRIME(POLINOMIO << (tam - 7), tam);
    //cout << "----------------- xor\n";
    
    // Faz a divisao com o xor entre os operandos
    crc ^= (POLINOMIO << (tam - 7));
    
    //IMPRIME(crc, tam);
    //cout << "\n";
  }
 
  //cout << "crc8: "; 
  //IMPRIME(crc, 8);
  //cout << "\n";
  return crc;
}

// Gera a tabela com os valores do crc
void gen_crc8_table()
{
  for ( int i = 0; i < 256; i++ )
    crc8_table[i] = calc_crc8_table(to_string(i));  
}

UI calc_crc8(string seq, UI res)
{
  UI crc;

  for ( char c : seq )
    crc = crc8_table[c];

  return crc;

}
