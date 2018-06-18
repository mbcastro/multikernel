#
# MIT License
#
# Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
# "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
# LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
#cflags += -DDEBUG
lflags := -Wl,--defsym=_LIBNOC_DISABLE_FIFO_FULL_CHECK=0
O := $(SRCDIR)/../output

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
rmem-server-lflags := -lmppaipc -pthread

#=============================================================================
# Mailbox Benchmark
#=============================================================================

cluster-bin += mailbox-slave
mailbox-slave-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
						   $(SRCDIR)/kernel/arch/mppa/name.c    \
						   $(SRCDIR)/kernel/arch/mppa/timer.c   \
						   $(SRCDIR)/kernel/arch/mppa/core.c    \
						   $(SRCDIR)/test/mailbox/slave.c

# Toolchain Configuration
mailbox-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL -DDEBUG_MAILBOX
mailbox-slave-lflags := -lmppaipc

io-bin += mailbox-master
mailbox-master-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
						   $(SRCDIR)/kernel/arch/mppa/name.c    \
							$(SRCDIR)/kernel/arch/mppa/core.c    \
							$(SRCDIR)/test/mailbox/master.c

# Toolchain Configuration
mailbox-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL -DDEBUG_MAILBOX
mailbox-master-lflags := -lmppaipc

mailbox-objs := rmem-server mailbox-master mailbox-slave
mailbox-name := mailbox.img

#=============================================================================
# MPPA Binary
#=============================================================================

mppa-bin := mailbox

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
