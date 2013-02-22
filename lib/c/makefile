SHELL = /bin/sh
TARGET = cfscc.so

SRCS = $(shell echo src/*.c)
OBJS = $(SRCS:.c=.o)

CFLAGS = -fPIC -Wall -Wextra
LDFLAGS = -shared

CC = gcc

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(TARGET) -lm

clean:
	rm src/*.o
	rm cfscc.so
