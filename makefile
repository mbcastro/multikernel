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

K1_TOOLCHAIN_DIR=/usr/local/k1tools/

# Directories.
BINDIR  = $(CURDIR)/bin
INCDIR  = $(CURDIR)/include
SRCDIR  = $(CURDIR)/src
TESTDIR = $(CURDIR)/test
BENCHDIR = $(CURDIR)/benchmark

# Toolchain configuration.
cflags := -ansi -std=c99
cflags += -Wall -Wextra
cflags += -O3 
cflags += -I $(INCDIR)
cflags += -D_KALRAY_MPPA256_
k1-lflags := -lmppaipc

#=============================================================================
# IO Cluster Binaries
#=============================================================================

io-bin := master.test rmem-server.test

master.test-srcs := $(TESTDIR)/master.c

rmem-server.test-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
						 $(SRCDIR)/kernel/arch/mppa/portal.c  \
						 $(SRCDIR)/kernel/arch/mppa/name.c    \
						 $(SRCDIR)/kernel/sys/timer.c         \
						 $(SRCDIR)/kernel/sys/memwrite.c      \
						 $(SRCDIR)/kernel/sys/memread.c       \
						 $(SRCDIR)/servers/rmem.c

#=============================================================================
# Compute Cluster Binaries
#=============================================================================

cluster-system := nodeos

cluster-bin := rmem

rmem-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
			 $(SRCDIR)/kernel/arch/mppa/portal.c  \
			 $(SRCDIR)/kernel/arch/mppa/name.c    \
			 $(SRCDIR)/kernel/sys/timer.c         \
			 $(SRCDIR)/kernel/sys/memwrite.c      \
			 $(SRCDIR)/kernel/sys/memread.c       \
			 $(TESTDIR)/rmem/rmem.c

#=============================================================================
# Testing Binary
#=============================================================================

test-objs := master.test      \
			 rmem-server.test \
			 rmem

test-name := test.img

#=============================================================================
# MPPA Binary
#=============================================================================

mppa-bin := test

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
