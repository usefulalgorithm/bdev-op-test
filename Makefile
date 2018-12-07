CC = g++
INCLUDE = include
CCFLAGS = -g -O3 -std=c++11
LIBS = -lstdc++fs
DEPS = SpookyV2.h
OBJ = main.o SpookyV2.o
SUBDIRS = stx-btree
TARGET = bdev-op-test

.PHONY: all

all: $(TARGET)

.PHONY: debug

debug: CCFLAGS += -DDEBUG -O0 -Wall
debug: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LIBS)
%.o: %.cc $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS) -I$(INCLUDE)

$(SUBDIRS):
	cd $(SUBDIRS) && ./configure && make

.PHONY: $(SUBDIRS)

.PHONY: clean

clean:
	@rm -f $(TARGET) *.o
