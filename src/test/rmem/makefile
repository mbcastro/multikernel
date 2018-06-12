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

#=============================================================================
# Kmeans Benchmark Kernel
#=============================================================================

cluster-bin += km-rmem-slave
km-rmem-slave-srcs := $(SRCDIR)/benchmark/km/rmem/slave/slave.c     \
					$(SRCDIR)/benchmark/km/rmem/slave/vector.c    \
					$(SRCDIR)/kernel/arch/mppa/mailbox.c \
					$(SRCDIR)/kernel/arch/mppa/noc.c     \
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
					$(SRCDIR)/kernel/arch/mppa/mailbox.c          \
					$(SRCDIR)/kernel/arch/mppa/noc.c              \
					$(SRCDIR)/kernel/arch/mppa/portal.c           \
					$(SRCDIR)/kernel/arch/mppa/barrier.c          \
					$(SRCDIR)/kernel/arch/mppa/name.c             \
					$(SRCDIR)/kernel/arch/mppa/core.c             \
					$(SRCDIR)/kernel/sys/meminit.c                \
					$(SRCDIR)/kernel/sys/memread.c                \
					$(SRCDIR)/kernel/sys/memwrite.c

# Toolchain Configuration
km-rmem-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
km-rmem-master-lflags := -lmppaipc -lm

km-rmem-objs := rmem-server km-rmem-master km-rmem-slave
km-rmem-name := km-rmem.img

#=============================================================================
# RMEM Latency Benchmark
#=============================================================================

cluster-bin += rmem-slave
rmem-slave-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
						$(SRCDIR)/kernel/arch/mppa/noc.c     \
						$(SRCDIR)/kernel/arch/mppa/portal.c  \
						$(SRCDIR)/kernel/arch/mppa/barrier.c \
						$(SRCDIR)/kernel/arch/mppa/name.c    \
						$(SRCDIR)/kernel/arch/mppa/timer.c   \
						$(SRCDIR)/kernel/arch/mppa/core.c    \
						$(SRCDIR)/kernel/sys/meminit.c       \
						$(SRCDIR)/kernel/sys/memread.c       \
						$(SRCDIR)/kernel/sys/memwrite.c      \
						$(SRCDIR)/test/rmem/slave.c

# Toolchain Configuration
rmem-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
rmem-slave-lflags := -lmppaipc

io-bin += rmem-master
rmem-master-srcs := $(SRCDIR)/kernel/arch/mppa/barrier.c \
							$(SRCDIR)/kernel/arch/mppa/core.c    \
							$(SRCDIR)/kernel/arch/mppa/name.c    \
							$(SRCDIR)/kernel/arch/mppa/mailbox.c \
							$(SRCDIR)/kernel/arch/mppa/noc.c     \
							$(SRCDIR)/test/rmem/master.c         \

# Toolchain Configuration
rmem-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
rmem-master-lflags := -lmppaipc

rmem-objs := rmem-server rmem-master rmem-slave
rmem-name := rmem.img

#=============================================================================
# Insertion Sort RMEM Benchmark Kernel
#=============================================================================

cluster-bin += is-rmem-slave
is-rmem-slave-srcs := $(SRCDIR)/benchmark/is/rmem/slave/slave.c    \
					$(SRCDIR)/benchmark/is/rmem/slave/sort.c   \
					$(SRCDIR)/kernel/arch/mppa/timer.c         \
					$(SRCDIR)/kernel/arch/mppa/mailbox.c       \
					$(SRCDIR)/kernel/arch/mppa/noc.c           \
					$(SRCDIR)/kernel/arch/mppa/barrier.c       \
					$(SRCDIR)/kernel/arch/mppa/portal.c        \
					$(SRCDIR)/kernel/arch/mppa/name.c          \
					$(SRCDIR)/kernel/arch/mppa/core.c          \
					$(SRCDIR)/kernel/sys/meminit.c             \
					$(SRCDIR)/kernel/sys/memread.c             \
					$(SRCDIR)/kernel/sys/memwrite.c
#$(SRCDIR)/benchmark/is/slave/ipc.c

# Toolchain Configuration
is-rmem-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
is-rmem-slave-cflags += -fopenmp
is-rmem-slave-lflags := -lmppaipc -lm -lgomp

io-bin += is-rmem-master
is-rmem-master-srcs := $(SRCDIR)/benchmark/is/rmem/master/main.c    \
					$(SRCDIR)/benchmark/is/rmem/master/bucketsort.c \
					$(SRCDIR)/benchmark/is/rmem/master/bucket.c     \
					$(SRCDIR)/benchmark/is/rmem/master/minibucket.c \
					$(SRCDIR)/benchmark/is/rmem/master/ipc.c        \
					$(SRCDIR)/benchmark/is/rmem/master/message.c    \
					$(SRCDIR)/benchmark/is/rmem/master/util.c       \
					$(SRCDIR)/kernel/arch/mppa/timer.c              \
					$(SRCDIR)/kernel/arch/mppa/mailbox.c            \
					$(SRCDIR)/kernel/arch/mppa/noc.c                \
					$(SRCDIR)/kernel/arch/mppa/portal.c             \
					$(SRCDIR)/kernel/arch/mppa/barrier.c            \
					$(SRCDIR)/kernel/arch/mppa/name.c               \
					$(SRCDIR)/kernel/arch/mppa/core.c               \
					$(SRCDIR)/kernel/sys/meminit.c                  \
					$(SRCDIR)/kernel/sys/memread.c                  \
					$(SRCDIR)/kernel/sys/memwrite.c

#$(SRCDIR)/benchmark/is/master/ipc.c

# Toolchain Configuration
is-rmem-master-system := rtems
is-rmem-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
is-rmem-master-lflags := -lmppaipc -pthread

is-rmem-objs := rmem-server is-rmem-master is-rmem-slave
is-rmem-name := is-rmem.img



#=============================================================================
# Gaussian Filter RMEM Benchmark Kernel
#=============================================================================

cluster-bin += gf-rmem-slave
gf-rmem-slave-srcs := $(SRCDIR)/benchmark/gf/rmem/slave/slave.c     \
					$(SRCDIR)/benchmark/gf/util.c        \
					$(SRCDIR)/kernel/arch/mppa/mailbox.c \
					$(SRCDIR)/kernel/arch/mppa/noc.c     \
					$(SRCDIR)/kernel/arch/mppa/portal.c  \
					$(SRCDIR)/kernel/arch/mppa/barrier.c \
					$(SRCDIR)/kernel/arch/mppa/name.c    \
					$(SRCDIR)/kernel/arch/mppa/timer.c   \
					$(SRCDIR)/kernel/arch/mppa/core.c    \
					$(SRCDIR)/kernel/sys/meminit.c       \
					$(SRCDIR)/kernel/sys/memread.c       \
					$(SRCDIR)/kernel/sys/memwrite.c

# Toolchain Configuration
gf-rmem-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
gf-rmem-slave-cflags += -fopenmp
gf-rmem-slave-lflags := -lmppaipc -lm -lgomp

io-bin += gf-rmem-master
gf-rmem-master-srcs := $(SRCDIR)/benchmark/gf/rmem/master/main.c \
					$(SRCDIR)/benchmark/gf/rmem/master/master.c \
					$(SRCDIR)/benchmark/gf/rmem/master/ipc.c    \
					$(SRCDIR)/benchmark/gf/util.c               \
					$(SRCDIR)/kernel/arch/mppa/mailbox.c        \
					$(SRCDIR)/kernel/arch/mppa/noc.c            \
					$(SRCDIR)/kernel/arch/mppa/portal.c         \
					$(SRCDIR)/kernel/arch/mppa/barrier.c        \
					$(SRCDIR)/kernel/arch/mppa/name.c           \
					$(SRCDIR)/kernel/arch/mppa/core.c           \
					$(SRCDIR)/kernel/sys/meminit.c              \
					$(SRCDIR)/kernel/sys/memread.c              \
					$(SRCDIR)/kernel/sys/memwrite.c

# Toolchain Configuration
gf-rmem-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
gf-rmem-master-lflags := -lmppaipc -lm

gf-rmem-objs := rmem-server gf-rmem-master gf-rmem-slave
gf-rmem-name := gf-rmem.img

#=============================================================================
# Gaussian Filter RMEM Benchmark Kernel - CHUNKS IN MASTER
#=============================================================================

cluster-bin += gf-dense-rmem-slave
gf-dense-rmem-slave-srcs := $(SRCDIR)/benchmark/gf/rmem-dense/slave/slave.c     \
					$(SRCDIR)/benchmark/gf/util.c        \
					$(SRCDIR)/kernel/arch/mppa/mailbox.c \
					$(SRCDIR)/kernel/arch/mppa/noc.c     \
					$(SRCDIR)/kernel/arch/mppa/portal.c  \
					$(SRCDIR)/kernel/arch/mppa/barrier.c \
					$(SRCDIR)/kernel/arch/mppa/name.c    \
					$(SRCDIR)/kernel/arch/mppa/timer.c   \
					$(SRCDIR)/kernel/arch/mppa/core.c    \
					$(SRCDIR)/kernel/sys/meminit.c       \
					$(SRCDIR)/kernel/sys/memread.c       \
					$(SRCDIR)/kernel/sys/memwrite.c

# Toolchain Configuration
gf-dense-rmem-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
gf-dense-rmem-slave-cflags += -fopenmp
gf-dense-rmem-slave-lflags := -lmppaipc -lm -lgomp

io-bin += gf-dense-rmem-master
gf-dense-rmem-master-srcs := $(SRCDIR)/benchmark/gf/rmem-dense/master/main.c \
					$(SRCDIR)/benchmark/gf/rmem-dense/master/master.c \
					$(SRCDIR)/benchmark/gf/rmem-dense/master/ipc.c    \
					$(SRCDIR)/benchmark/gf/util.c                     \
					$(SRCDIR)/kernel/arch/mppa/mailbox.c              \
					$(SRCDIR)/kernel/arch/mppa/noc.c                  \
					$(SRCDIR)/kernel/arch/mppa/portal.c               \
					$(SRCDIR)/kernel/arch/mppa/barrier.c              \
					$(SRCDIR)/kernel/arch/mppa/name.c                 \
					$(SRCDIR)/kernel/arch/mppa/core.c                 \
					$(SRCDIR)/kernel/sys/meminit.c                    \
					$(SRCDIR)/kernel/sys/memread.c                    \
					$(SRCDIR)/kernel/sys/memwrite.c

# Toolchain Configuration
gf-dense-rmem-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
gf-dense-rmem-master-lflags := -lmppaipc -lm

gf-dense-rmem-objs := rmem-server gf-dense-rmem-master gf-dense-rmem-slave
gf-dense-rmem-name := gf-dense-rmem.img

#=============================================================================
# MPPA Binary
#=============================================================================

mppa-bin := rmem is-rmem km-rmem gf-rmem gf-dense-rmem

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
