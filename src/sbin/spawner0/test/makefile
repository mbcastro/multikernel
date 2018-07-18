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
.PHONY:master

#===============================================================================

# Builds automated test driver.
all: master slaves

# Builds Main Binary
master:
ifeq ($(BUILDING_IOCLUSTER),true)
	cd master && $(MAKE) all
endif

# Builds Slave Binaries
slaves: 
ifeq ($(BUILDING_CCLUSTER),true)
	cd hal/sync && $(MAKE) all
	cd hal/mailbox && $(MAKE) all
	cd hal/portal && $(MAKE) all
	cd ipc/barrier && $(MAKE) all
	cd ipc/mailbox && $(MAKE) all
	cd ipc/name && $(MAKE) all
endif

# Cleans compilation files.
clean:
	cd master && $(MAKE) clean
	cd hal/sync && $(MAKE) clean
	cd hal/mailbox && $(MAKE) clean
	cd hal/portal && $(MAKE) clean
	cd ipc/barrier && $(MAKE) clean
	cd ipc/mailbox && $(MAKE) clean
	cd ipc/name && $(MAKE) clean

# Cleans everything.
distclean: clean
	cd master && $(MAKE) distclean
	cd hal/sync && $(MAKE) distclean
	cd hal/mailbox && $(MAKE) distclean
	cd hal/portal && $(MAKE) distclean
	cd ipc/barrier && $(MAKE) distclean
	cd ipc/mailbox && $(MAKE) distclean
	cd ipc/name && $(MAKE) distclean

