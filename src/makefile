#
# MIT License
#
# Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
# "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
# LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

# Import Platform-Dependent Stuff
ifeq ($(ARCH),$(filter $(ARCH),k1bdp k1bio))
	include $(MAKEDIR)/makefile.mppa256
endif

#===============================================================================
# Toolchain
#===============================================================================

# Compiler Options
export CFLAGS += -ansi -std=c99
export CFLAGS += -Wall -Wextra -Werror
export CFLAGS += -Winit-self -Wswitch-default -Wfloat-equal
export CFLAGS += -Wundef -Wshadow -Wuninitialized
export CFLAGS += -O3
export CFLAGS += -I $(INCDIR)
ifdef DEBUG
export CFLAGS += -DDEBUG
endif

# Linker Options
export LDFLAGS += 
export ARFLAGS = rcs

#===============================================================================
# Binaries & Libraries
#===============================================================================

# Startup Routines
export CRT0 = $(LIBDIR)/crt0.o
export CRT1 = $(LIBDIR)/crt1.o

# Servers
export LIBSERVERS = $(LIBDIR)/libservers.a

# System Services
export SERVERSBIN1 = $(BINDIR)/servers1

#===============================================================================

# Conflicts.
.PHONY: kernel
.PHONY: libs
.PHONY: servers
.PHONY: sbin

# Builds everything.
all: kernel libs servers sbin benchmarks

# Builds the kernel.
kernel: clean
	cd kernel && $(MAKE) all

# Builds user-level libraries.
libs: kernel
	mkdir -p $(LIBDIR)
	cd libs && $(MAKE) all

# Builds system servers
servers: kernel libs
ifeq ($(BUILD_SERVERS),true)
	cd servers && $(MAKE) all
endif

# Builds binaries.
sbin: kernel servers libs
	mkdir -p $(BINDIR)
	cd sbin && $(MAKE) all

# Builds benchmarks.
benchmarks: kernel servers libs
	mkdir -p $(BINDIR)
	cd sbin/benchmarks && $(MAKE) all

# Cleans compilation files.
clean:
	cd kernel && $(MAKE) clean
	cd servers && $(MAKE) clean
	cd libs && $(MAKE) clean
	cd sbin && $(MAKE) clean
	cd sbin && $(MAKE) clean

# Cleans everything.
distclean:
	cd kernel && $(MAKE) distclean
	cd servers && $(MAKE) distclean
	cd libs && $(MAKE) distclean
	cd sbin && $(MAKE) distclean
	cd sbin && $(MAKE) distclean

