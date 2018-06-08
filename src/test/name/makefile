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
# Name server test
#=============================================================================

cluster-bin += name-slave
name-slave-srcs := $(SRCDIR)/kernel/arch/mppa/barrier.c \
							 $(SRCDIR)/kernel/arch/mppa/core.c    \
							 $(SRCDIR)/kernel/arch/mppa/mailbox.c \
							 $(SRCDIR)/kernel/arch/mppa/name.c \
						   $(SRCDIR)/test/name/slave.c

# Toolchain Configuration
name-slave-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
name-slave-lflags := -lmppaipc -pthread

io-bin += name-master
name-master-srcs := $(SRCDIR)/kernel/arch/mppa/barrier.c \
							$(SRCDIR)/kernel/arch/mppa/mailbox.c \
							$(SRCDIR)/kernel/arch/mppa/name.c \
							$(SRCDIR)/kernel/arch/mppa/core.c    \
							$(SRCDIR)/test/name/master.c

# Toolchain Configuration
name-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
name-master-lflags := -lmppaipc

name-objs := name-server name-master name-slave
name-name := name.img

#=============================================================================
# MPPA Binary
#=============================================================================

mppa-bin := name

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
