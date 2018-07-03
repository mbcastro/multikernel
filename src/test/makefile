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
.PHONY: hal-sync
.PHONY: name
.PHONY: mailbox
.PHONY: barrier

# Change binaries directory.
export BINDIR := $(BINDIR)/test

# Builds everything.
all: hal-sync name mailbox barrier

# Builds HAL Sync testing unit.
hal-sync:
	mkdir -p $(BINDIR)
	cd hal-sync && $(MAKE) all

# Builds Naming Service testing unit
name:
	mkdir -p $(BINDIR)
	cd name && $(MAKE) all

# Builds Barrier testing unit
barrier:
	mkdir -p $(BINDIR)
	cd barrier && $(MAKE) all

# Cleans compilation files.
clean:
	cd hal-sync && $(MAKE) clean
	cd name && $(MAKE) clean
	cd mailbox && $(MAKE) clean
	cd barrier && $(MAKE) clean

# Cleans everything.
distclean:
	cd hal-sync && $(MAKE) distclean
	cd name && $(MAKE) distclean
	cd mailbox && $(MAKE) distclean
	cd barrier && $(MAKE) distclean
