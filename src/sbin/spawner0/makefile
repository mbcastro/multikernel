#
# MIT License
#
# Copyright (c) 211-218 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
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

# C Source Files
export INIT_SRC = $(wildcard $(CURDIR)/init/hal/core/*.c)      \
                  $(wildcard $(CURDIR)/init/hal/sync/*.c)      \
                  $(wildcard $(CURDIR)/init/hal/mailbox/*.c)   \
                  $(wildcard $(CURDIR)/init/hal/portal/*.c)    \
                  $(wildcard $(CURDIR)/init/ipc/barrier/*.c)   \
                  $(wildcard $(CURDIR)/init/ipc/name/*.c)      \
                  $(wildcard $(CURDIR)/init/ipc/mailbox/*.c)   \
                  $(wildcard $(CURDIR)/init/ipc/portal/*.c)    \
                  $(wildcard $(CURDIR)/init/ipc/semaphore/*.c) \
                  $(wildcard $(CURDIR)/init/main.c)

# Conflicts
.PHONY: test

#===============================================================================

# Builds everything.
all: test benchmark

# Builds Test-Driver Utilitiy.
test:
	cd test && $(MAKE) all

# Builds Synthetic Benchmarks.
benchmark:
	cd rmem-benchmark && $(MAKE) all

# Cleans compilation files.
clean:
	cd test && $(MAKE) clean
	cd rmem-benchmark && $(MAKE) clean

# Cleans everything.
distclean: clean
	cd test && $(MAKE) distclean
	cd rmem-benchmark && $(MAKE) distclean
