TARGET = brainfuck
LIBS = 
CC = gcc
CFLAGS = -std=c99 -g -Wextra -Wall -Werror -Wno-unused-variable -Wimplicit-fallthrough 

.PHONY: default all

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(CFLAGS) $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f *.tmp
	-rm -f $(TARGET)
