# Como utilizar a bibliota

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
fq.get_dado();

```

## Impressao

```cpp
f1.imprime();

```

Saida:

```
Frame 1
Ini:  01111110
Tipo: 010000
Seq:  0010
Tam:  000001
Dado: 1
Crc8: 01111000

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
O arquivo de teste eh o frame.cpp, execute:

```
make
./frame
```
