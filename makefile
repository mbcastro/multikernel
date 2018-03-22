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

io-bin := noc-latency-master

#master.test-srcs := $(TESTDIR)/master.c          \
#					$(SRCDIR)/kernel/sys/timer.c \
#					$(SRCDIR)/kernel/arch/mppa/barrier.c \

#rmem-server.test-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
#						 $(SRCDIR)/kernel/arch/mppa/portal.c  \
#						 $(SRCDIR)/kernel/arch/mppa/barrier.c \
#						 $(SRCDIR)/kernel/arch/mppa/name.c    \
#						 $(SRCDIR)/kernel/sys/timer.c         \
#						 $(SRCDIR)/kernel/sys/meminit.c       \
#						 $(SRCDIR)/kernel/sys/memwrite.c      \
#						 $(SRCDIR)/kernel/sys/memread.c       \
#						 $(SRCDIR)/servers/rmem.c

noc-latency-master-srcs := $(SRCDIR)/kernel/sys/timer.c     \
						   $(BENCHDIR)/noc-latency/master.c 

#=============================================================================
# Compute Cluster Binaries
#=============================================================================

cluster-bin := noc-latency-slave

#rmem-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
#			 $(SRCDIR)/kernel/arch/mppa/portal.c  \
#			 $(SRCDIR)/kernel/arch/mppa/barrier.c \
#			 $(SRCDIR)/kernel/arch/mppa/name.c    \
#			 $(SRCDIR)/kernel/sys/timer.c         \
#			 $(SRCDIR)/kernel/sys/meminit.c       \
#			 $(SRCDIR)/kernel/sys/memwrite.c      \
#			 $(SRCDIR)/kernel/sys/memread.c       \
#			 $(TESTDIR)/rmem/rmem.c

noc-latency-slave-srcs := $(BENCHDIR)/noc-latency/slave.c \
						  $(SRCDIR)/kernel/sys/timer.c

#=============================================================================
# Testing Binary
#=============================================================================

#test-objs := master.test      \
#			 rmem-server.test \
#			 rmem 

#test-name := test.img

#=============================================================================
# Benchmark Binary
#=============================================================================

noc-latency-objs := noc-latency-master noc-latency-slave

noc-latency-name := noc-latency.img

#=============================================================================
# MPPA Binary
#=============================================================================

mppa-bin := noc-latency

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
