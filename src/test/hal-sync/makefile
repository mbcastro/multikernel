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
# Synchronization Point Unit Test
#=============================================================================

io-bin += hal-sync-master
hal-sync-master-srcs := $(SRCDIR)/test/hal-sync/master.c     \
                        $(SRCDIR)/kernel/arch/mppa/setup.c   \
                        $(SRCDIR)/kernel/arch/mppa/core.c    \
                        $(SRCDIR)/kernel/arch/mppa/mailbox.c \
                        $(SRCDIR)/kernel/arch/mppa/sync.c    \
                        $(SRCDIR)/kernel/arch/mppa/noc.c

# Toolchain Configuration
hal-sync-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL -DDEBUG_MAILBOX
hal-sync-master-lflags := -lmppaipc

hal-sync-objs := hal-sync-master 
hal-sync-name := hal-sync.img

#=============================================================================
# MPPA Binary
#=============================================================================

mppa-bin := hal-sync

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
