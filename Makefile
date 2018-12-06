TARGET = bdev-op-test
CC = g++
INCLUDE = include
CCFLAGS = -g -O3 -std=c++11

all: $(TARGET)

debug: CCFLAGS += -DDEBUG -g -O0 -Wall
debug: $(TARGET)

$(TARGET): main.o
	$(CC) -o $(TARGET) main.o
main.o: main.cc
	$(CC) -I$(INCLUDE) $(CCFLAGS) -c main.cc
clean:
	@rm -f $(TARGET) *.o
