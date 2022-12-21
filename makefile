CC = g++
CFLAGS = -g -std=c++20 
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

# O nome do target deve ser o nome do arquivo principal
<<<<<<< HEAD
TARGET = msg
=======
TARGET = exemplo
>>>>>>> 4efe292 (atualizei o gitignore a add o makefile)

all: $(TARGET)

$(TARGET): $(OBJ)

clean:
	rm -f *.o vgcore* 

purge: clean
	rm -f $(TARGET)
