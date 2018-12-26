CC = g++
INCLUDE = ./include
CCFLAGS = -g -O3 -std=c++11
LDFLAGS += -Wl,-rpath=./lib -L./lib
LIBS = -lboost_filesystem
DEPS = SpookyV2.h common.h utils.h cache.h daemon.h
OBJ = SpookyV2.o common.o utils.o cache.o daemon.o main.o
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
