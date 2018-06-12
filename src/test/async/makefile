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
# Async Latency Benchmark
#=============================================================================

io-bin += master.elf
master.elf-srcs := $(SRCDIR)/test/async/master.c

# Toolchain Configuration
master.elf-system := bare
master.elf-cflags += -D_KALRAY_MPPA_256_LOW_LEVEL
master.elf-lflags := -mhypervisor -lutask -lmppa_async -lmppa_request_engine
master.elf-lflags += -lmppapower -lmppanoc -lmpparouting
master.elf-lflags += -lpcie_queue

cluster-bin += slave.elf
slave.elf-srcs := $(SRCDIR)/test/async/slave.c \
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
# MPPA Binary
#=============================================================================

mppa-bin := async

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
