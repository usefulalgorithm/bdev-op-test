TARGET = bdev-op-test
CC = g++
INCLUDE = include
CCFLAGS = -g -O3 -std=c++11

.PHONY: all

all: $(TARGET)

.PHONY: debug

debug: CCFLAGS += -DDEBUG -O0 -Wall
debug: $(TARGET)

$(TARGET): main.o
	$(CC) -o $(TARGET) main.o
main.o: main.cc
	$(CC) -I$(INCLUDE) $(CCFLAGS) -c main.cc

.PHONY: clean

clean:
	@rm -f $(TARGET) *.o
