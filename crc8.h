#ifndef _CRC8_
#define _CRC8_

#include <string>
#include "macros.h"

using namespace std;

// Gera a tabela com os valores do crc para um determinado polinomio
void gen_crc8_table();

// Calcula o crc8 de uma string, para imprimir o calculo de CRC descomente
uint8_t calc_crc8(uint8_t *msg, int tamByte);

#endif
