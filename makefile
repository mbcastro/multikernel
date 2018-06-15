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
export OUTDIR  = $(CURDIR)/output

export LIBNAME = libkernel.a

#===============================================================================
# Toolchain
#===============================================================================

# Toochain Location
export TOOLCHAIN=/usr/local/k1tools

# Toolchain
export CC = $(TOOLCHAIN)/bin/k1-gcc
export LD = $(TOOLCHAIN)/bin/k1-ld
export AR = $(TOOLCHAIN)/bin/k1-ar

# Compiler Options
export CFLAGS = -ansi -std=c99
export CFLAGS += -Wall -Wextra -Werror
export CFLAGS += -Winit-self -Wswitch-default -Wfloat-equal
export CFLAGS += -Wundef -Wshadow -Wuninitialized
export CFLAGS += -O3
export CFLAGS += -I $(INCDIR)
export CFLAGS += -D_KALRAY_MPPA256 -D_KALRAY_MPPA_256_HIGH_LEVEL
ifdef DEBUG
export CFLAGS += -DDEBUG
endif

# Linker Options
export LDFLAGS = -Wl,--defsym=_LIBNOC_DISABLE_FIFO_FULL_CHECK=0 -O=essai
export ARFLAGS = rcs

#===============================================================================
# Libraries
#===============================================================================

# MPPA IPC
export LIBMPPAIPC = $(TOOLCHAIN)/k1-rtems/lib/libmppaipc.a

# Kernel
export LIBKERNEL = $(LIBDIR)/libkernel.a

#===============================================================================

# Builds everything.
all: kernel test

# Builds the kernel.
kernel:
	cd $(SRCDIR) && $(MAKE) kernel

# Builds testing system.
test: kernel
	cd $(SRCDIR) && $(MAKE) test

# Cleans compilation files.
clean:
	cd $(SRCDIR) && $(MAKE) clean
