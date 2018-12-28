CC = g++
INCLUDE = ./include
CCFLAGS = -g -O3 -std=c++11
LDFLAGS += -Wl,-rpath=./lib -L./lib
LIBS = -lboost_filesystem
DEPS = $(wildcard src/*.h)
SRC = $(wildcard src/*.cc)
OBJ = $(SRC:.cc=.o)
TARGET = bdev-op-test

.PHONY: all

all: $(TARGET)

.PHONY: debug

debug: CCFLAGS += -DDEBUG -O0 -Wall

debug: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LDFLAGS) $(LIBS)
%.o: %.cc $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS) -I$(INCLUDE)

.PHONY: clean

clean:
	@rm -f $(TARGET) $(OBJ)
