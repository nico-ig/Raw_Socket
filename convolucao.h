#ifndef _CONV_
#define _CONV_

// Gera a tabela de convolucao (1/2) para os polinomio p1 e p2. Por mais q o 
// parametro seja um inteiro, os polinomios devem ter 3 bits 
void gen_conv_table(int p1, int p2);

// Retorna a convolucao de uma determinada string
std::vector<uint16_t> set_conv(std::string d);

#endif
