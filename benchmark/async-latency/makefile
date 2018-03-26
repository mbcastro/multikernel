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

# Toolchain configuration.
cflags := -ansi -std=c99
cflags += -Wall -Wextra -Werror
cflags += -Winit-self -Wswitch-default -Wfloat-equal -Wundef -Wshadow -Wuninitialized
cflags += -O3 
cflags += -I $(INCDIR)
cflags += -D_KALRAY_MPPA256_ -DMODE_STANDALONE

# Cluster rules
cluster-bin := slave.elf
slave.elf-srcs := slave.c
slave.elf-system := bare
slave.elf-cflags :=  -mhypervisor
slave.elf-lflags := -mhypervisor -Wl,--defsym=USER_STACK_SIZE=0x2000 -Wl,--defsym=KSTACK_SIZE=0x1000
slave.elf-lflags += -lvbsp -lutask -lmppa_remote -lmppa_async -lmppa_request_engine -lmppapower -lmppanoc -lmpparouting -Wl,--defsym=_LIBNOC_DISABLE_FIFO_FULL_CHECK=0

io-bin := master.elf
master.elf-srcs := master.c
master.elf-system := bare
master.elf-cflags := -mhypervisor
master.elf-lflags := -lvbsp -lmppa_remote -lmppa_async -lmppa_request_engine -lutask  -lmppapower -lmppanoc -lmpparouting -lpcie_queue -mhypervisor -Wl,--defsym=_LIBNOC_DISABLE_FIFO_FULL_CHECK=0

async-latency-objs := master.elf slave.elf
async-latency-name := async-latency.img
mppa-bin := async-latency

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray

run_jtag: all
	${TRACE_CMD_PREFIX} $(K1_TOOLCHAIN_DIR)/bin/k1-jtag-runner ${JTAG_ARGS} --multibinary=./${O}/bin/${mppa-bin}.img --exec-multibin=IODDR0:${io-bin}
