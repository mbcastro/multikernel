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

io-bin += hal-master
hal-master-srcs := $(SRCDIR)/kernel/arch/mppa/core.c \
                   $(SRCDIR)/test/hal/master.c

# Toolchain Configuration
hal-master-cflags += -D_KALRAY_MPPA_256_HIGH_LEVEL
hal-master-lflags := -lmppaipc

hal-objs := hal-master 
hal-name := hal.img

#=============================================================================
# MPPA Binary
#=============================================================================

mppa-bin := hal

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
