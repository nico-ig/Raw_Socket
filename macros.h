#ifndef _MACROS_
#define _MACROS_

#define UC unsigned char
#define UI unsigned int

// Macro que cria uma sequencia com o 1 deslocado "des" posicoes a direita
#define MASKR(des) (1 << des)

// Macro que fica em 1 ou 0 de acordo com o valor do bit avaliado
#define BIT(value, des) ( (value & MASKR(des)) ? 1 : 0 )

// Macro que zera uma sequencia de bits
#define ZERA(value) ( value &= ~value )

// Macro que coloca todos os bits de uma sequencia em 1
#define UM(seq) ( seq |= ~seq )

// Macro que zera determinado bit de uma sequencia
#define ZERABIT(seq, bit) ( seq &= ~MASKR(bit) )

// Macro para a "p" potencia de 2
#define BITPOW(p) ( MASKR(6) )

// Macro que imprime uma sequencia de bits
#define IMPRIME(seq, tam) for ( int i = tam-1; i >= 0; i-- ) \
                            cout << BIT(seq, i) << " "; \
                          cout << "\n";
#endif
