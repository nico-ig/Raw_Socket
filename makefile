CC = g++
CPPFLAGS = -g -std=c++20 
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)


# O nome do target deve ser o nome do arquivo principal
TARGET = exemplo

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CPPFLAGS) -o $(TARGET) $(OBJ)

clean:
	rm -f src/*.o vgcore* 

purge: clean
	rm -f $(TARGET)
