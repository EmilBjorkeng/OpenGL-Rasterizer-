CC = g++
CFLAGS = -Isrc/include -std=c++26 -Wall -Wextra
LDFLAGS = -Lsrc/lib
GLFWFLAGS = -lglfw3

TARGET = main
SRC = $(filter-out src/$(TARGET).cpp, $(wildcard src/*.cpp))
OBJ = $(SRC:src/%.cpp=%.o) glad.o

.PHONY: all clean run

all: $(TARGET)

%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

glad.o: src/gl.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ src/$(TARGET).cpp $(LDLIBS) ${GLFWFLAGS} -o $@

clean:
	-del $(TARGET).exe 2>nul || true
	-del *.o 2>nul || true

run: clean all
	.\$(TARGET).exe
	$(MAKE) clean