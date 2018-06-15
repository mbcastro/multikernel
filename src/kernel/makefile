#
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

# Additional Compiler Opitions
CFLAGS += -march=k1b -mos=rtems -mcluster=ioddr 

# C source Files
SRC = $(wildcard arch/mppa/*.c) \
      $(wildcard ipc/*.c)

# Object Files
OBJ = $(SRC:.c=.o)

#===============================================================================

# Builds the kernel.
all: $(OBJ)
	mkdir -p $(LIBDIR)
	$(AR) $(ARFLAGS) $(LIBKERNEL) $(OBJ)

# Builds object files.
%.o: %.c
	$(CC) $(CFLAGS) $<  -c -o $@

# Cleans compilation files.
clean:
	rm -rf $(OBJ) $(LIBDIR)/$(LIBNAME)
