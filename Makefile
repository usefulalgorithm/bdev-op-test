TARGET = bdev-op-test
CC = g++
CFLAGS = -g -O3 -Wall

all: $(TARGET)
$(TARGET): main.o
	$(CC) -o $(TARGET) main.o
main.o: main.cc
	$(CC) -c main.cc
clean:
	rm -f $(TARGET) *.o
