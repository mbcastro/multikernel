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

#===============================================================================
# Directories
#===============================================================================

# Directories.
export BINDIR  = $(CURDIR)/bin
export INCDIR  = $(CURDIR)/include
export LIBDIR  = $(CURDIR)/lib
export SRCDIR  = $(CURDIR)/src

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

include $(CURDIR)/build/makefile.mppa256

# System Services
export SERVERSBIN = $(BINDIR)/servers

#===============================================================================

# Builds everything.
all: kernel test servers

# Builds the kernel.
kernel:
	cd $(SRCDIR) && $(MAKE) kernel

# Builds system servers
servers: kernel
	cd $(SRCDIR) && $(MAKE) servers

# Builds testing system.
test: kernel servers
	cd $(SRCDIR) && $(MAKE) test

# Builds system image.
image:
	cd $(SRCDIR) && $(MAKE) image

# Cleans compilation files.
clean:
	cd $(SRCDIR) && $(MAKE) clean
