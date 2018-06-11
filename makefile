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

export K1_TOOLCHAIN_DIR=/usr/local/k1tools/

# Directories.
export BINDIR  = $(CURDIR)/bin
export INCDIR  = $(CURDIR)/include
export SRCDIR  = $(CURDIR)/src
export OUTDIR  = $(CURDIR)/output

# Toolchain configuration.
export cflags := -ansi -std=c99
export cflags += -Wall -Wextra -Werror
export cflags += -Winit-self -Wswitch-default -Wfloat-equal -Wundef -Wshadow -Wuninitialized
export cflags += -O3
export cflags += -I $(INCDIR)
export cflags += -D_KALRAY_MPPA256
ifdef DEBUG
export cflags += -DDEBUG
endif
export lflags := -Wl,--defsym=_LIBNOC_DISABLE_FIFO_FULL_CHECK=0 -O=essai
export O := $(OUTDIR)

#=============================================================================
# Servers
#=============================================================================

export io-bin += name-server

# Name Server
export name-server-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
							$(SRCDIR)/kernel/arch/mppa/barrier.c \
							$(SRCDIR)/kernel/arch/mppa/name.c    \
							$(SRCDIR)/kernel/arch/mppa/core.c    \
							$(SRCDIR)/kernel/arch/mppa/noc.c     \
							$(SRCDIR)/servers/name.c

export name-server-system := rtems
export name-server-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
export name-server-lflags := -lmppaipc -pthread

# RMEM Server
export rmem-server-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
							$(SRCDIR)/kernel/arch/mppa/portal.c  \
							$(SRCDIR)/kernel/arch/mppa/barrier.c \
							$(SRCDIR)/kernel/arch/mppa/name.c    \
							$(SRCDIR)/kernel/arch/mppa/core.c    \
							$(SRCDIR)/kernel/arch/mppa/noc.c     \
							$(SRCDIR)/servers/rmem.c

export rmem-server-system := rtems
export rmem-server-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
export rmem-server-lflags := -lmppaipc -pthread

#=============================================================================

all: hal-mailbox name 

async:
	cd $(CURDIR)/src/test/async/ && $(MAKE);

hal-mailbox:
	cd $(CURDIR)/src/test/hal-mailbox/ && $(MAKE);

mailbox:
	cd $(CURDIR)/src/test/mailbox/ && $(MAKE);

name:
	cd $(CURDIR)/src/test/name/ && $(MAKE);

portal:
	cd $(CURDIR)/src/test/portal/ && $(MAKE);

rmem:
	cd $(CURDIR)/src/test/rmem/ && $(MAKE);

clean:
	cd $(CURDIR)/src/test/async/; make clean;
	cd $(CURDIR)/src/test/hal-mailbox/; make clean;
	cd $(CURDIR)/src/test/mailbox/; make clean;
	cd $(CURDIR)/src/test/name/; make clean;
	cd $(CURDIR)/src/test/portal/; make clean;
	cd $(CURDIR)/src/test/rmem/; make clean;
