CC = g++
CXXFLAGS = -Isrc/include -std=c++26 -Wall -Wextra
PKG_CFLAGS := $(shell pkg-config --cflags glfw3)
PKG_LDFLAGS := $(shell pkg-config --static --libs glfw3)

TARGET = Main
SRC_CPP = $(wildcard src/*.cpp)
SRC_C = $(wildcard src/*.c)
OBJ = $(SRC_CPP:src/%.cpp=%.o) $(SRC_C:src/%.c=%.o)

ifeq ($(OS),Windows_NT)
	RM = del /Q
	EXE = .exe
	RUN_CMD = .\$(TARGET)$(EXE)
else
	RM = rm -f
	EXE =
	RUN_CMD = ./$(TARGET)
endif

.PHONY: all clean run

all: $(TARGET)

%.o: src/%.cpp
	$(CC) $(CXXFLAGS) $(PKG_CFLAGS) -c $< -o $@

%.o: src/%.c
	$(CC) $(CXXFLAGS) $(PKG_CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CXXFLAGS) $(PKG_LDFLAGS) $^ -o $@$(EXE)

clean:
	$(RM) $(TARGET)$(EXE)
	$(RM) *.o

run: all
	$(RUN_CMD)
