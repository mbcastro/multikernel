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
hal-master-srcs := $(SRCDIR)/kernel/arch/mppa/core.c  \
                   $(SRCDIR)/kernel/arch/mppa/noc.c   \
                   $(SRCDIR)/kernel/arch/mppa/setup.c \
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

# C source files.
SRC = $(wildcard *.c)

# Object files.
OBJ = $(SRC:.c=.o)

# Builds the kernel.
all: $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) -march=k1b -mos=rtems -mcluster=ioddr $(OBJ) -o $(BINDIR)/hal-master $(LIBDIR)/$(LIBNAME) $(TOOLCHAIN)/k1-rtems/lib/libmppaipc.a
	$(TOOLCHAIN)/bin/k1-create-multibinary --remove-prefix $(BINDIR) --boot $(BINDIR)/hal-master -T $(BINDIR)/hal.img

# Builds object file.
%.o: %.c
	$(CC) -march=k1b -mos=rtems -mcluster=ioddr $< $(CFLAGS) -c -o $@

# Cleans compilation files.
clean:
	rm -rf $(OBJ) $(BINDIR)/hal-master
	rm -rf $(OBJ) $(BINDIR)/hal.img
	rm -rf $(OBJ) $(LIBDIR)/$(LIBNAME)
