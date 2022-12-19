# Como utilizar a bibliota

## Clang format
Para usar o clang formatter e manter seu código padronizado, siga os seguintes passos:
> 1 - primeiro instale o clang-format na sua distribuição linux

> 2 - antes de fazer um commit execute:
```bash
 clang-fomat -i codigos.c # ou *.c para modificar todos, também funciona para *.cpp
 ```

> OBS: caso queira deixar a formatação automática, siga o tutorial para o seu editor de texto:
https://clang.llvm.org/docs/ClangFormat.html
---
# Montar um frame padrao de testes
---
# Montagem de frame
- Definir as estruturas pro frame
- Descobrir o tipo
- Descobrir o tamanho do Data
- Transformar a msg em bits
- Calcular o check sum para o frame
- Montar o frame
---
# Desmontagem de frame
- Verificar o marcador de inicio
- Decodificar o tipo
- Descobrir a sequencia
- Ver o tamanho
- Ler o dado (final eh o tamanho lido)
- Concatenar os frames()
---
# Concatenar os frames
- Decodificar o dado
- Exibir a mensagem
---
# Envio do dado
- Verificar se deu erro
- Verificar se recebeu ack
- Deslizar a janela
- Em caso de erro ver o que reenviar
- Timeout
---
# Receber o dado
- Enviar o nack, ack, e qual frame reenviar
- Veririficar o frame
- Conferir o check sum
- Desmontar o frame()
---
# Erros
- Ver se o marcador de inicio eh valido
- Ver se eh um tipo valido
- Implementar mensagens de erros
- Ver os casos de erros
---
# Log
---
# Interface do usuario
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
