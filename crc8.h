#ifndef _CRC8_
#define _CRC8_

#include <string>
#include "macros.h"

using namespace std;

// Imprime a tabela para do crc para um determinado polinomio
void print_crc8_table();

// Gera a tabela com os valores do crc para um determinado polinomio
void gen_crc8_table();

// Calcula o crc8 de uma string, para imprimir o calculo de CRC descomente
UI calc_crc8(string seq, UI res);

#endif
