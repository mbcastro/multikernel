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

# Conflicts.
.PHONY: crt0
.PHONY: crt1
.PHONY: libc
.PHONY: nanvix

export CFLAGS += -I $(INCDIR)

#===============================================================================

# Builds startup routines.
all: crt0 crt1 nanvix libc

# Builds startup routines for IO Cluster.
crt0: nanvix libc
ifeq ($(BUILDING_IOCLUSTER),true)
	cd crt0 && $(MAKE) all
endif

# Builds startup routines for Compute Cluster.
crt1: nanvix libc
ifeq ($(BUILDING_CCLUSTER),true)
	cd crt1 && $(MAKE) all
endif

# Builds C library.
libc: nanvix
	cd libc && $(MAKE) all

# Builds Nanvix library.
nanvix:
	cd nanvix && $(MAKE) all

# Cleans compilation files.
clean:
	cd crt0 && $(MAKE) clean
	cd crt1 && $(MAKE) clean
	cd libc && $(MAKE) clean
	cd nanvix && $(MAKE) clean

# Cleans everything.
distclean: clean
	cd crt0 && $(MAKE) distclean
	cd crt1 && $(MAKE) distclean
	cd libc && $(MAKE) distclean
	cd nanvix && $(MAKE) distclean

