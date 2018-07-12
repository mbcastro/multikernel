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

# System Services
export SERVERSBIN1 = $(BINDIR)/servers1

#===============================================================================

# Conflicts.
.PHONY: benchmarks
.PHONY: kernel
.PHONY: libs
.PHONY: servers
.PHONY: tests

# Builds everything.
all: kernel servers libs tests benchmarks 

# Builds the kernel.
kernel: clean
ifeq ($(BUILD_KERNEL),true)
	cd kernel && $(MAKE) all
endif

# Builds system servers
servers: kernel
ifeq ($(BUILD_SERVERS),true)
	cd servers && $(MAKE) all
endif

# Builds user-level libraries.
libs: kernel
	mkdir -p $(LIBDIR)
	cd libs && $(MAKE) all

# Builds testing system.
tests: kernel servers libs
ifeq ($(BUILD_TESTS),true)
	mkdir -p $(BINDIR)
	cd tests && $(MAKE) all
endif

# Builds benchmarks.
benchmarks: kernel servers libs
ifeq ($(BUILD_BENCHMARKS),true)
	mkdir -p $(BINDIR)
	cd benchmarks && $(MAKE) all
endif

# Cleans compilation files.
clean:
	cd kernel && $(MAKE) clean
	cd servers && $(MAKE) clean
	cd libs && $(MAKE) clean
	cd tests && $(MAKE) clean
	cd benchmarks && $(MAKE) clean

# Cleans everything.
distclean:
	cd kernel && $(MAKE) distclean
	cd servers && $(MAKE) distclean
	cd libs && $(MAKE) distclean
	cd tests && $(MAKE) distclean
	cd benchmarks && $(MAKE) distclean

