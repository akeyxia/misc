# Makefile for USB tools

CROSS_COMPILE=arm-linux-gnueabihf-

CC = $(CROSS_COMPILE)gcc
PTHREAD_LIBS = -lpthread
WARNINGS = -Wall -Wextra
CFLAGS = $(WARNINGS) -g $(PTHREAD_LIBS)

all: testusb
%: %.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) testusb
