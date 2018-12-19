CC = g++
INCLUDE = ./include
CCFLAGS = -g -O3 -std=c++11
LDFLAGS += -Wl,-rpath=./lib -L./lib
LIBS = -lstdc++fs -lboost_iostreams
DEPS = SpookyV2.h cache.h common.h utils.h
OBJ = SpookyV2.o cache.o main.o 
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
	@rm -f $(TARGET) *.o
