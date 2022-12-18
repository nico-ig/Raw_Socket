
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
