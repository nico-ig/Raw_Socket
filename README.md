# Como utilizar a bibliota de FRAMES

O frame pode ser inicializado direto e depois adicionando os dados(1) 
ou passando como parametro o tipo, a sequencia e uma string de dado (2)

```cpp
frame f1();
frame f2(0x01, 0x01, "1");

```

## Adicionar dados

Para adicionar os dados no frame, existem tres funcoes de set: 

- set\_tipo()
- set\_seq()
- set\_dado()

Para utilizar, o tipo e a sequencia sao valores inteiros, e o dado uma string

```cpp
f1.set_tipo(0x01);
f1.set_seq(0x01);
f1.set_dado("1");

```
Um dado com mais deo que 63 bits nao sera adicionado, a funcao retorna

## Pegar os dados

Para pegar os valores do frame, existem tres funcoes de get:

- get\_tipo()
- get\_seq()
- get\_dado()

Exemplo:

```cpp
f1.get_tipo();
f1.get_seq();
f1.get_dado();

```

## CRC8

O crc eh calculado ao criar o frame. Para realizar a verificacao, utilize a 
funcao chk\_crc8(). Ela retorna 1 caso seja valido e 0 caso seja invalido.
Eh necessario chamar a funcao gen\_crc8\_table() no inicio do programa, para
que seja possivel realizar o calculo do crc8

```cpp
gen_crc8_table();
f1.chk_crc8() ? cout << "valido" : cout << "invalido";
```

## Impressao

Os dados podem ser impressos na forma binaria, hexadecimal ou decimal

```cpp
f1.imprime(BIN);
f1.imprime(HEX);
f1.imprime(DEC);
```

Saida:

```
 Ini: 01111110
Tipo: 000001
 Seq: 0001
 Tam: 00000001
Dado: 00110000
Crc8: 01010110

 Ini: 0x7E
Tipo: 0x1
 Seq: 0x1
 Tam: 0x1
Dado: 0x30
Crc8: 0x56

 Ini: 126
Tipo: 1
 Seq: 1
 Tam: 1
Dado: 0
Crc8: 86
```

## Tipos de frame
A biblioteca possui alguns tipos definidos no header

- TEXTO
- MIDIA
- ACK
- NACK
- ERRO
- INIT (inicio de transmissao)
- FIMT (fim de transmissao)
- DADOS

## Chamada
O arquivo de teste eh o exemplo.cpp, execute:

```
make
./exemplo
```
