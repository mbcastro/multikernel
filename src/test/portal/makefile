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
BINDIR  = $(CURDIR)/../../../bin
INCDIR  = $(CURDIR)/../../../include
SRCDIR  = $(CURDIR)/../../

# Toolchain Configuration
cflags := -ansi -std=c99
cflags += -Wall -Wextra -Werror
cflags += -Winit-self -Wswitch-default -Wfloat-equal -Wundef -Wshadow -Wuninitialized
cflags += -O3
cflags += -I $(INCDIR)
cflags += -D_KALRAY_MPPA256
cflags += -DDEBUG
lflags := -Wl,--defsym=_LIBNOC_DISABLE_FIFO_FULL_CHECK=0
O := $(SRCDIR)/../output

#=============================================================================
# Name server
#=============================================================================

io-bin += name-server
name-server-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
					$(SRCDIR)/kernel/arch/mppa/barrier.c \
					$(SRCDIR)/kernel/arch/mppa/name.c \
					$(SRCDIR)/kernel/arch/mppa/core.c    \
					$(SRCDIR)/servers/name.c

# Toolchain Configuration
name-server-system := rtems
name-server-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
name-server-lflags := -lmppaipc -pthread

#=============================================================================
# Kmeans Benchmark Kernel
#=============================================================================

cluster-bin += km-portal-slave
km-portal-slave-srcs := $(SRCDIR)/benchmark/km/portal/slave/slave.c     \
					 $(SRCDIR)/benchmark/km/portal/slave/vector.c    \
					 $(SRCDIR)/benchmark/km/portal/slave/ipc.c    \
					 $(SRCDIR)/kernel/arch/mppa/portal.c  \
					 $(SRCDIR)/kernel/arch/mppa/mailbox.c  \
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
					  $(SRCDIR)/kernel/arch/mppa/portal.c  \
						$(SRCDIR)/kernel/arch/mppa/mailbox.c  \
					  $(SRCDIR)/kernel/arch/mppa/name.c    \
					  $(SRCDIR)/kernel/arch/mppa/core.c

# Toolchain Configuration
km-portal-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
km-portal-master-lflags := -lmppaipc -lm

km-portal-objs := km-portal-master km-portal-slave name-server
km-portal-name := km-portal.img

#=============================================================================
# Portal Latency Benchmark
#=============================================================================

io-bin += portal-master
portal-master-srcs := $(SRCDIR)/test/portal/master.c  \
					  $(SRCDIR)/kernel/arch/mppa/portal.c  \
						$(SRCDIR)/kernel/arch/mppa/mailbox.c  \
					  $(SRCDIR)/kernel/arch/mppa/name.c    \
					  $(SRCDIR)/kernel/arch/mppa/core.c

# Toolchain Configuration
portal-master-system := rtems
portal-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
portal-master-lflags := -lmppaipc

cluster-bin += portal-slave
portal-slave-srcs := $(SRCDIR)/test/portal/slave.c   \
					 $(SRCDIR)/kernel/arch/mppa/portal.c  \
					 $(SRCDIR)/kernel/arch/mppa/mailbox.c  \
					 $(SRCDIR)/kernel/arch/mppa/barrier.c \
					 $(SRCDIR)/kernel/arch/mppa/name.c    \
					 $(SRCDIR)/kernel/arch/mppa/timer.c   \
					 $(SRCDIR)/kernel/arch/mppa/core.c

# Toolchain Configuration
portal-slave-system := nodeos
portal-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
portal-slave-lflags := -lmppaipc

portal-objs := portal-master portal-slave name-server
portal-name := portal.img

#=============================================================================
# Insertion Sort Portal Benchmark Kernel
#=============================================================================

cluster-bin += is-portal-slave
is-portal-slave-srcs := $(SRCDIR)/benchmark/is/portal/slave/slave.c    \
					 $(SRCDIR)/benchmark/is/portal/slave/sort.c   \
					 $(SRCDIR)/kernel/arch/mppa/timer.c  \
					 $(SRCDIR)/kernel/arch/mppa/portal.c  \
					 $(SRCDIR)/kernel/arch/mppa/mailbox.c  \
					 $(SRCDIR)/kernel/arch/mppa/name.c    \
					 $(SRCDIR)/kernel/arch/mppa/core.c
#$(SRCDIR)/benchmark/is/slave/ipc.c

# Toolchain Configuration
is-portal-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
is-portal-slave-cflags += -fopenmp
is-portal-slave-lflags := -lmppaipc -lm -lgomp

io-bin += is-portal-master
is-portal-master-srcs := $(SRCDIR)/benchmark/is/portal/master/main.c    \
					  $(SRCDIR)/benchmark/is/portal/master/bucketsort.c   \
					  $(SRCDIR)/benchmark/is/portal/master/bucket.c \
					  $(SRCDIR)/benchmark/is/portal/master/minibucket.c \
					  $(SRCDIR)/benchmark/is/portal/master/ipc.c  \
					  $(SRCDIR)/benchmark/is/portal/master/message.c \
					  $(SRCDIR)/benchmark/is/portal/master/util.c    \
					  $(SRCDIR)/kernel/arch/mppa/timer.c  \
					  $(SRCDIR)/kernel/arch/mppa/portal.c  \
						$(SRCDIR)/kernel/arch/mppa/mailbox.c  \
					  $(SRCDIR)/kernel/arch/mppa/name.c    \
					  $(SRCDIR)/kernel/arch/mppa/core.c


#$(SRCDIR)/benchmark/is/master/ipc.c

# Toolchain Configuration
is-portal-master-system := rtems
is-portal-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
is-portal-master-lflags := -lmppaipc -pthread

is-portal-objs := is-portal-master is-portal-slave name-server
is-portal-name := is-portal.img

#=============================================================================
# Gaussian Filter Portal Benchmark Kernel
#=============================================================================

cluster-bin += gf-portal-slave
gf-portal-slave-srcs := $(SRCDIR)/benchmark/gf/portal/slave/slave.c \
				        $(SRCDIR)/benchmark/gf/portal/slave/ipc.c   \
						 		$(SRCDIR)/benchmark/gf/util.c               \
				        $(SRCDIR)/kernel/arch/mppa/timer.c          \
				        $(SRCDIR)/kernel/arch/mppa/portal.c         \
								$(SRCDIR)/kernel/arch/mppa/mailbox.c  \
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
						 $(SRCDIR)/benchmark/gf/util.c                 \
						 $(SRCDIR)/kernel/arch/mppa/timer.c            \
						 $(SRCDIR)/kernel/arch/mppa/portal.c           \
						 $(SRCDIR)/kernel/arch/mppa/mailbox.c  \
						 $(SRCDIR)/kernel/arch/mppa/name.c             \
						 $(SRCDIR)/kernel/arch/mppa/core.c

# Toolchain Configuration
gf-portal-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
gf-portal-master-lflags := -lmppaipc -lm

gf-portal-objs := gf-portal-master gf-portal-slave name-server
gf-portal-name := gf-portal.img

#=============================================================================
# MPPA Binary
#=============================================================================

mppa-bin := portal is-portal km-portal gf-portal

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
