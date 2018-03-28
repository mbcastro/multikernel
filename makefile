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
BENCHDIR = $(CURDIR)/benchmark

# Toolchain Configuration
cflags := -ansi -std=c99
cflags += -Wall -Wextra -Werror
cflags += -Winit-self -Wswitch-default -Wfloat-equal -Wundef -Wshadow -Wuninitialized
cflags += -O3 
cflags += -I $(INCDIR)
cflags += -D_KALRAY_MPPA256
lflags := -Wl,--defsym=_LIBNOC_DISABLE_FIFO_FULL_CHECK=0

#=============================================================================
# Nanvix Kernel
#=============================================================================

io-bin += rmem-server
rmem-server-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
					$(SRCDIR)/kernel/arch/mppa/portal.c  \
					$(SRCDIR)/kernel/arch/mppa/barrier.c \
					$(SRCDIR)/kernel/arch/mppa/name.c    \
					$(SRCDIR)/servers/rmem.c

# Toolchain Configuration
rmem-server-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
rmem-server-system := rtems
rmem-server-lflags := -lmppaipc -pthread

#=============================================================================
# Async Latency Benchmark
#=============================================================================

io-bin += master.elf
master.elf-srcs := $(BENCHDIR)/async-latency/master.c

# Toolchain Configuration
master.elf-system := bare
master.elf-cflags += -D_KALRAY_MPPA_256_LOW_LEVEL
master.elf-lflags := -mhypervisor -lutask -lmppa_async -lmppa_request_engine
master.elf-lflags += -lmppapower -lmppanoc -lmpparouting
master.elf-lflags += -lpcie_queue

cluster-bin += slave.elf
slave.elf-srcs := $(BENCHDIR)/async-latency/slave.c \
				  $(SRCDIR)/kernel/arch/mppa/timer.c

# Toolchain Configuration
slave.elf-system := bare
slave.elf-cflags += -D_KALRAY_MPPA_256_LOW_LEVEL
slave.elf-lflags := -mhypervisor -lutask -lmppa_async -lmppa_request_engine
slave.elf-lflags += -lmppapower -lmppanoc -lmpparouting
slave.elf-lflags += -Wl,--defsym=USER_STACK_SIZE=0x2000
slave.elf-lflags += -Wl,--defsym=KSTACK_SIZE=0x1000

async-latency-objs := master.elf slave.elf
async-latency-name := async-latency.img

#=============================================================================
# Portal Latency Benchmark
#=============================================================================

io-bin += portal-latency-master
portal-latency-master-srcs := $(BENCHDIR)/portal-latency/master.c \
							  $(SRCDIR)/kernel/arch/mppa/timer.c

# Toolchain Configuration
portal-latency-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
portal-latency-master-lflags := -lmppaipc

cluster-bin += portal-latency-slave
portal-latency-slave-srcs := $(BENCHDIR)/portal-latency/slave.c

# Toolchain Configuration
portal-latency-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
portal-latency-slave-lflags := -lmppaipc

portal-latency-objs := portal-latency-master portal-latency-slave
portal-latency-name := portal-latency.img

#=============================================================================
# RMEM Latency Benchmark
#=============================================================================

cluster-bin += rmem-latency-slave
rmem-latency-slave-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
						   $(SRCDIR)/kernel/arch/mppa/portal.c  \
						   $(SRCDIR)/kernel/arch/mppa/barrier.c \
						   $(SRCDIR)/kernel/arch/mppa/name.c    \
						   $(SRCDIR)/kernel/arch/mppa/timer.c   \
						   $(SRCDIR)/kernel/sys/meminit.c       \
						   $(SRCDIR)/kernel/sys/memread.c       \
						   $(SRCDIR)/kernel/sys/memwrite.c      \
						   $(BENCHDIR)/rmem-latency/slave.c

# Toolchain Configuration
rmem-latency-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
rmem-latency-slave-lflags := -lmppaipc

io-bin += rmem-latency-master
rmem-latency-master-srcs := $(SRCDIR)/kernel/arch/mppa/barrier.c \
							$(BENCHDIR)/rmem-latency/master.c

# Toolchain Configuration
rmem-latency-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
rmem-latency-master-lflags := -lmppaipc

rmem-latency-objs := rmem-server rmem-latency-slave rmem-latency-master
rmem-latency-name := rmem-latency.img

#=============================================================================
# MPPA Binary
#=============================================================================

mppa-bin := portal-latency async-latency rmem-latency

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
