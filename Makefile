TARGET = bdev-op-test
CC = g++
INCLUDE = include
CFLAGS = -g -O0 -Wall -std=c++11

all: $(TARGET)
$(TARGET): main.o
	$(CC) -o $(TARGET) main.o
main.o: main.cc
	$(CC) -I$(INCLUDE) $(CFLAGS) -c main.cc
clean:
	@rm -f $(TARGET) *.o
