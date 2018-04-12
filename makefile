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

# Toolchain Configuration
cflags := -ansi -std=c99
cflags += -Wall -Wextra -Werror
cflags += -Winit-self -Wswitch-default -Wfloat-equal -Wundef -Wshadow -Wuninitialized
cflags += -O3
cflags += -I $(INCDIR)
cflags += -D_KALRAY_MPPA256 -DDEBUG
lflags := -Wl,--defsym=_LIBNOC_DISABLE_FIFO_FULL_CHECK=0

#=============================================================================
# Nanvix Kernel
#=============================================================================

io-bin += rmem-server
rmem-server-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
					$(SRCDIR)/kernel/arch/mppa/portal.c  \
					$(SRCDIR)/kernel/arch/mppa/barrier.c \
					$(SRCDIR)/kernel/arch/mppa/name.c    \
					$(SRCDIR)/kernel/arch/mppa/core.c    \
					$(SRCDIR)/servers/rmem.c

# Toolchain Configuration
rmem-server-system := rtems
rmem-server-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
rmem-server-system := rtems
rmem-server-lflags := -lmppaipc -pthread

#=============================================================================
# Async Latency Benchmark
#=============================================================================

io-bin += master.elf
master.elf-srcs := $(SRCDIR)/benchmark/async/master.c

# Toolchain Configuration
master.elf-system := bare
master.elf-cflags += -D_KALRAY_MPPA_256_LOW_LEVEL
master.elf-lflags := -mhypervisor -lutask -lmppa_async -lmppa_request_engine
master.elf-lflags += -lmppapower -lmppanoc -lmpparouting
master.elf-lflags += -lpcie_queue

cluster-bin += slave.elf
slave.elf-srcs := $(SRCDIR)/benchmark/async/slave.c \
				  $(SRCDIR)/kernel/arch/mppa/core.c    \
				  $(SRCDIR)/kernel/arch/mppa/timer.c

# Toolchain Configuration
slave.elf-system := bare
slave.elf-cflags += -D_KALRAY_MPPA_256_LOW_LEVEL
slave.elf-lflags := -mhypervisor -lutask -lmppa_async -lmppa_request_engine
slave.elf-lflags += -lmppapower -lmppanoc -lmpparouting
slave.elf-lflags += -Wl,--defsym=USER_STACK_SIZE=0x2000
slave.elf-lflags += -Wl,--defsym=KSTACK_SIZE=0x1000

async-objs := master.elf slave.elf
async-name := async.img

#=============================================================================
# Portal Latency Benchmark
#=============================================================================

io-bin += portal-master
portal-master-srcs := $(SRCDIR)/benchmark/portal/master.c  \
					  $(SRCDIR)/kernel/arch/mppa/portal.c  \
					  $(SRCDIR)/kernel/arch/mppa/name.c    \
					  $(SRCDIR)/kernel/arch/mppa/core.c

# Toolchain Configuration
portal-master-system := rtems
portal-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
portal-master-lflags := -lmppaipc

cluster-bin += portal-slave
portal-slave-srcs := $(SRCDIR)/benchmark/portal/slave.c   \
					 $(SRCDIR)/kernel/arch/mppa/portal.c  \
					 $(SRCDIR)/kernel/arch/mppa/barrier.c \
					 $(SRCDIR)/kernel/arch/mppa/name.c    \
					 $(SRCDIR)/kernel/arch/mppa/timer.c   \
					 $(SRCDIR)/kernel/arch/mppa/core.c

# Toolchain Configuration
portal-slave-system := nodeos
portal-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
portal-slave-lflags := -lmppaipc

portal-objs := portal-master portal-slave
portal-name := portal.img

#=============================================================================
# Mailbox Benchmark
#=============================================================================

cluster-bin += mailbox-slave
mailbox-slave-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
						   $(SRCDIR)/kernel/arch/mppa/name.c    \
						   $(SRCDIR)/kernel/arch/mppa/timer.c   \
						   $(SRCDIR)/kernel/arch/mppa/core.c    \
						   $(SRCDIR)/benchmark/mailbox/slave.c

# Toolchain Configuration
mailbox-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL -DDEBUG_MAILBOX
mailbox-slave-lflags := -lmppaipc

io-bin += mailbox-master
mailbox-master-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
						   $(SRCDIR)/kernel/arch/mppa/name.c    \
							$(SRCDIR)/kernel/arch/mppa/core.c    \
							$(SRCDIR)/benchmark/mailbox/master.c

# Toolchain Configuration
mailbox-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL -DDEBUG_MAILBOX
mailbox-master-lflags := -lmppaipc

mailbox-objs := rmem-server mailbox-master mailbox-slave 
mailbox-name := mailbox.img

#=============================================================================
# RMEM Latency Benchmark
#=============================================================================

cluster-bin += rmem-slave
rmem-slave-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
						   $(SRCDIR)/kernel/arch/mppa/portal.c  \
						   $(SRCDIR)/kernel/arch/mppa/barrier.c \
						   $(SRCDIR)/kernel/arch/mppa/name.c    \
						   $(SRCDIR)/kernel/arch/mppa/timer.c   \
						   $(SRCDIR)/kernel/arch/mppa/core.c    \
						   $(SRCDIR)/kernel/sys/meminit.c       \
						   $(SRCDIR)/kernel/sys/memread.c       \
						   $(SRCDIR)/kernel/sys/memwrite.c      \
						   $(SRCDIR)/benchmark/rmem/slave.c

# Toolchain Configuration
rmem-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
rmem-slave-lflags := -lmppaipc

io-bin += rmem-master
rmem-master-srcs := $(SRCDIR)/kernel/arch/mppa/barrier.c \
							$(SRCDIR)/kernel/arch/mppa/core.c    \
							$(SRCDIR)/benchmark/rmem/master.c

# Toolchain Configuration
rmem-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
rmem-master-lflags := -lmppaipc

rmem-objs := rmem-server rmem-master rmem-slave
rmem-name := rmem.img

#=============================================================================
# Kmeans Benchmark Kernel
#=============================================================================

cluster-bin += km-rmem-slave
km-rmem-slave-srcs := $(SRCDIR)/benchmark/km/rmem/slave/slave.c     \
					 $(SRCDIR)/benchmark/km/rmem/slave/vector.c    \
					 $(SRCDIR)/kernel/arch/mppa/mailbox.c \
					 $(SRCDIR)/kernel/arch/mppa/portal.c  \
					 $(SRCDIR)/kernel/arch/mppa/barrier.c \
					 $(SRCDIR)/kernel/arch/mppa/name.c    \
					 $(SRCDIR)/kernel/arch/mppa/timer.c   \
					 $(SRCDIR)/kernel/arch/mppa/core.c    \
					 $(SRCDIR)/kernel/sys/meminit.c       \
					 $(SRCDIR)/kernel/sys/memread.c       \
					 $(SRCDIR)/kernel/sys/memwrite.c

# Toolchain Configuration
km-rmem-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
km-rmem-slave-cflags += -fopenmp
km-rmem-slave-lflags := -lmppaipc -lm -lgomp

io-bin += km-rmem-master
km-rmem-master-srcs := $(SRCDIR)/benchmark/km/rmem/master/main.c     \
					  $(SRCDIR)/benchmark/km/rmem/master/master.c   \
					  $(SRCDIR)/benchmark/km/rmem/master/vector.c   \
					  $(SRCDIR)/benchmark/km/rmem/master/util.c     \
					  $(SRCDIR)/benchmark/km/rmem/master/ipc.c      \
					  $(SRCDIR)/kernel/arch/mppa/mailbox.c \
					  $(SRCDIR)/kernel/arch/mppa/portal.c  \
					  $(SRCDIR)/kernel/arch/mppa/barrier.c \
					  $(SRCDIR)/kernel/arch/mppa/name.c    \
					  $(SRCDIR)/kernel/arch/mppa/timer.c   \
					  $(SRCDIR)/kernel/arch/mppa/core.c    \
					  $(SRCDIR)/kernel/sys/meminit.c       \
					  $(SRCDIR)/kernel/sys/memread.c       \
					  $(SRCDIR)/kernel/sys/memwrite.c
 
# Toolchain Configuration
km-rmem-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
km-rmem-master-lflags := -lmppaipc -lm

km-rmem-objs := rmem-server km-rmem-master km-rmem-slave
km-rmem-name := km-rmem.img


cluster-bin += km-portal-slave
km-portal-slave-srcs := $(SRCDIR)/benchmark/km/portal/slave/slave.c     \
					 $(SRCDIR)/benchmark/km/portal/slave/vector.c    \
					 $(SRCDIR)/benchmark/km/portal/slave/ipc.c    \
					 $(SRCDIR)/kernel/arch/mppa/mailbox.c \
					 $(SRCDIR)/kernel/arch/mppa/portal.c  \
					 $(SRCDIR)/kernel/arch/mppa/barrier.c \
					 $(SRCDIR)/kernel/arch/mppa/name.c    \
					 $(SRCDIR)/kernel/arch/mppa/timer.c   \
					 $(SRCDIR)/kernel/arch/mppa/core.c

# Toolchain Configuration
km-portal-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
km-portal-slave-cflags += -fopenmp
km-portal-slave-lflags := -lmppaipc -lm -lgomp

io-bin += km-portal-master
km-portal-master-srcs := $(SRCDIR)/benchmark/km/portal/master/main.c     \
					  $(SRCDIR)/benchmark/km/portal/master/master.c   \
					  $(SRCDIR)/benchmark/km/portal/master/vector.c   \
					  $(SRCDIR)/benchmark/km/portal/master/util.c     \
					  $(SRCDIR)/benchmark/km/portal/master/ipc.c      \
					  $(SRCDIR)/kernel/arch/mppa/mailbox.c \
					  $(SRCDIR)/kernel/arch/mppa/portal.c  \
					  $(SRCDIR)/kernel/arch/mppa/barrier.c \
					  $(SRCDIR)/kernel/arch/mppa/name.c    \
					  $(SRCDIR)/kernel/arch/mppa/timer.c   \
					  $(SRCDIR)/kernel/arch/mppa/core.c
 
# Toolchain Configuration
km-portal-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
km-portal-master-lflags := -lmppaipc -lm

km-portal-objs := km-portal-master km-portal-slave
km-portal-name := km-portal.img

#=============================================================================
# Insertion_sort Benchmark Kernel
#=============================================================================

cluster-bin += is-slave
is-slave-srcs := $(SRCDIR)/benchmark/is/slave/slave.c    \
					 $(SRCDIR)/kernel/arch/mppa/mailbox.c     \
					 $(SRCDIR)/benchmark/is/slave/sort.c   \
					 $(SRCDIR)/kernel/arch/mppa/timer.c  \
					 $(SRCDIR)/kernel/arch/mppa/portal.c  \
					 $(SRCDIR)/kernel/arch/mppa/name.c    \
					 $(SRCDIR)/kernel/arch/mppa/core.c    
#$(SRCDIR)/benchmark/is/slave/ipc.c

# Toolchain Configuration
is-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
is-slave-cflags += -fopenmp
is-slave-lflags := -lmppaipc -lm -lgomp

io-bin += is-master
is-master-srcs := $(SRCDIR)/benchmark/is/master/main.c    \
					  $(SRCDIR)/kernel/arch/mppa/mailbox.c     \
					  $(SRCDIR)/benchmark/is/master/bucketsort.c   \
					  $(SRCDIR)/benchmark/is/master/bucket.c \
					  $(SRCDIR)/benchmark/is/master/minibucket.c \
					  $(SRCDIR)/kernel/arch/mppa/timer.c  \
					  $(SRCDIR)/kernel/arch/mppa/portal.c  \
					  $(SRCDIR)/kernel/arch/mppa/name.c    \
					  $(SRCDIR)/kernel/arch/mppa/core.c    \
					  $(SRCDIR)/benchmark/is/master/ipc.c  \
					  $(SRCDIR)/benchmark/is/master/message.c \
					  $(SRCDIR)/benchmark/is/master/util.c    


#$(SRCDIR)/benchmark/is/master/ipc.c     

# Toolchain Configuration
is-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
is-master-cflags += -lpthread
is-master-lflags := -lmppaipc -lm

is-objs := rmem-server is-master is-slave 
is-name := is.img

#=============================================================================
# Gaussian Filter Portal Benchmark Kernel
#=============================================================================

cluster-bin += gf-portal-slave
gf-portal-slave-srcs := $(SRCDIR)/benchmark/gf/portal/slave/slave.c \
				        $(SRCDIR)/benchmark/gf/portal/slave/ipc.c   \
				        $(SRCDIR)/kernel/arch/mppa/timer.c          \
				        $(SRCDIR)/kernel/arch/mppa/portal.c         \
				        $(SRCDIR)/kernel/arch/mppa/name.c           \
				        $(SRCDIR)/kernel/arch/mppa/core.c

# Toolchain Configuration
gf-portal-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
gf-portal-slave-cflags += -fopenmp
gf-portal-slave-lflags := -lmppaipc -lm -lgomp

io-bin += gf-portal-master
gf-portal-master-srcs := $(SRCDIR)/benchmark/gf/portal/master/main.c   \
						 $(SRCDIR)/benchmark/gf/portal/master/master.c \
						 $(SRCDIR)/benchmark/gf/portal/master/ipc.c    \
						 $(SRCDIR)/benchmark/gf/portal/master/util.c   \
						 $(SRCDIR)/kernel/arch/mppa/timer.c            \
						 $(SRCDIR)/kernel/arch/mppa/portal.c           \
						 $(SRCDIR)/kernel/arch/mppa/name.c             \
						 $(SRCDIR)/kernel/arch/mppa/core.c
 
# Toolchain Configuration
gf-portal-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
gf-portal-master-lflags := -lmppaipc -lm

gf-portal-objs := rmem-server gf-portal-master gf-portal-slave
gf-portal-name := gf-portal.img

#=============================================================================
# MPPA Binary
#=============================================================================

mppa-bin := portal async mailbox rmem is km-rmem km-portal gf-portal

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
