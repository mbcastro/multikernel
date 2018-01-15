# Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
# 
# This file is part of Nanvix.
# 
# Nanvix is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# Nanvix is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
#

MACHINE = UNIX

# Directories.
BINDIR  = $(CURDIR)/bin
LIBDIR  = $(CURDIR)/lib
INCDIR  = $(CURDIR)/include
SRCDIR  = $(CURDIR)/src
TESTDIR = $(CURDIR)/test
BENCHDIR = $(CURDIR)/benchmark

# Toolchain.
LD = gcc
CC = gcc
MPICC = mpicc

# Toolchain configuration.
CFLAGS += -ansi -std=c99
CFLAGS += -Wall -Wextra
CFLAGS += -O3 -D $(MACHINE)
CFLAGS += -I $(INCDIR)

SRC = $(wildcard $(SRCDIR)/kernel/arch/unix/*.c)         \
	  $(wildcard $(SRCDIR)/kernel/klib/*.c)  \
	  $(wildcard $(SRCDIR)/kernel/pm/*.c) \
	  $(wildcard $(SRCDIR)/lib/syscall/*.c) \

all: bdev ramdisk ipc.test memwrite.benchmark memread.benchmark

bdev: $(SRC) $(SRCDIR)/kernel/dev/bdev.c
	mkdir -p $(BINDIR)
	$(LD) $(CFLAGS) $^ -o $(BINDIR)/bdev

ramdisk: $(SRC) $(SRCDIR)/kernel/dev/block/ramdisk.c
	mkdir -p $(BINDIR)
	$(LD) $(CFLAGS) $^ -o $(BINDIR)/ramdisk

memread.benchmark: $(SRC) $(BENCHDIR)/memread.c
	mkdir -p $(BINDIR)
	$(MPICC) $(CFLAGS) $^ -o $(BINDIR)/memread.benchmark

memwrite.benchmark: $(SRC) $(BENCHDIR)/memwrite.c
	mkdir -p $(BINDIR)
	$(MPICC) $(CFLAGS) $^ -o $(BINDIR)/memwrite.benchmark

ipc.test: $(SRC) $(TESTDIR)/ipc.c
	mkdir -p $(BINDIR)
	$(LD) $(CFLAGS) $^ -o $(BINDIR)/ipc.test
	
# Builds object file from C source file.
%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

# Cleans compilation files.
clean:
	rm -rf $(BINDIR)/*
