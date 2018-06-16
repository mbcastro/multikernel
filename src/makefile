#
# Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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
export SERVERSBIN = $(BINDIR)/servers

#===============================================================================

# Conflicts.
.PHONY: kernel
.PHONY: servers
.PHONY: test

# Builds everything.
all: kernel tests servers

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

# Builds testing system.
tests: kernel servers
ifeq ($(BUILD_TESTS),true)
	cd test && $(MAKE) all
endif

# Cleans compilation files.
clean:
	cd kernel && $(MAKE) clean
	cd servers && $(MAKE) clean
	cd test && $(MAKE) clean

# Cleans everything.
distclean:
	cd kernel && $(MAKE) distclean
	cd servers && $(MAKE) distclean
	cd test && $(MAKE) distclean

