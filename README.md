# Log

No inicio do programa, criar o arquivo de log. Por padrao, o arquivo eh salvo na home, mas pode ser especificado o caminho onde deseja salvar o arquivo.

```cpp
log arqLog1(".log");
log arqLog2("./", ".log");
```

O primeiro será salvo na home e o segundo no diretorio atual.

## Adicionando entradas ao log

Para adicionar uma linha ao log, chame a funcao add\_log com a string desejada. Nao eh nescessario adicionar um "\n", pois a funcao fara isso.

```cpp
arqLog.add_log("Esta eh uma linha do log");
```
