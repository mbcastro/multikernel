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
.PHONY:nanvix
.PHONY:posix

#===============================================================================

# Builds automated test driver.
all:  nanvix posix

# Nanvix API test drivers.
nanvix:
	cd nanvix/kernel/sync && $(MAKE) all
	cd nanvix/kernel/mailbox && $(MAKE) all
	cd nanvix/kernel/portal && $(MAKE) all
	cd nanvix/ipc/barrier && $(MAKE) all
	cd nanvix/ipc/name && $(MAKE) all
	cd nanvix/ipc/mailbox && $(MAKE) all
	cd nanvix/ipc/portal&& $(MAKE) all
	cd nanvix/mm/rmem && $(MAKE) all

# POSIX API test drivers.
posix:
	cd posix/semaphore && $(MAKE) all

# Cleans compilation files.
clean:
	cd nanvix/kernel/sync && $(MAKE) clean
	cd nanvix/kernel/mailbox && $(MAKE) clean
	cd nanvix/kernel/portal && $(MAKE) clean
	cd nanvix/ipc/barrier && $(MAKE) clean
	cd nanvix/ipc/name && $(MAKE) clean
	cd nanvix/ipc/mailbox && $(MAKE) clean
	cd nanvix/ipc/portal && $(MAKE) clean
	cd nanvix/mm/rmem && $(MAKE) clean
	cd posix/semaphore && $(MAKE) clean

# Cleans everything.
distclean: clean
	cd nanvix/kernel/sync && $(MAKE) distclean
	cd nanvix/kernel/mailbox && $(MAKE) distclean
	cd nanvix/kernel/portal && $(MAKE) distclean
	cd nanvix/ipc/barrier && $(MAKE) distclean
	cd nanvix/ipc/name && $(MAKE) distclean
	cd nanvix/ipc/mailbox && $(MAKE) distclean
	cd nanvix/ipc/portal && $(MAKE) distclean
	cd nanvix/mm/rmem && $(MAKE) distclean
	cd posix/semaphore && $(MAKE) distclean

