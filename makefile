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

# Toolchain configuration.
cflags := -ansi -std=c99
cflags += -Wall -Wextra
cflags += -O3
cflags += -I $(INCDIR)
cflags += -D_KALRAY_MPPA256_
k1-lflags := -lmppaipc

io-bin := master
master-srcs := $(TESTDIR)/master.c

cluster-bin := noc.test
noc.test-srcs := $(SRCDIR)/kernel/arch/mppa/noc.c $(TESTDIR)/noc.c

cluster-bin := mailbox.test
mailbox.test-srcs := $(SRCDIR)/kernel/arch/mppa/noc.c $(SRCDIR)/kernel/pm/mailbox.c $(TESTDIR)/mailbox.c

test-objs := master noc.test mailbox.test
test-name := test.img

mppa-bin := test

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
