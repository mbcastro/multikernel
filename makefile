TARGET = TARGET_UNIX

# Directories.
BINDIR  = $(CURDIR)/bin
INCDIR  = $(CURDIR)/include
SRCDIR  = $(CURDIR)/src
TESTDIR = $(CURDIR)/test


CC = gcc

CLFAGS += -ansi -std=c99
CFLAGS += -Wall -Wextra
CFLAGS += -I $(INCDIR) -D$(TARGET)

all: ramdisk

ramdisk:
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(SRCDIR)/ramdisk/*.c $(SRCDIR)/pm/*.c -o $(BINDIR)/ramdisk
	$(CC) $(CFLAGS) $(TESTDIR)/*.c $(SRCDIR)/pm/*.c -o $(BINDIR)/ramdisk.test

# Cleans compilation files.
clean:
	rm -rf $(BINDIR)/ramdisk
	rm -rf $(BINDIR)/ramdisk.test
