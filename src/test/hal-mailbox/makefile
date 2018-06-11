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
# Mailbox Unit Test
#=============================================================================

io-bin += hal-mailbox-master
hal-mailbox-master-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
                           $(SRCDIR)/kernel/arch/mppa/core.c    \
                           $(SRCDIR)/kernel/arch/mppa/noc.c     \
                           $(SRCDIR)/test/hal-mailbox/master.c

# Toolchain Configuration
hal-mailbox-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL -DDEBUG_MAILBOX
hal-mailbox-master-lflags := -lmppaipc

hal-mailbox-objs := hal-mailbox-master 
hal-mailbox-name := hal-mailbox.img

#=============================================================================
# MPPA Binary
#=============================================================================

mppa-bin := hal-mailbox

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
