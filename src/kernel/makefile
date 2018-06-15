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

# C source Files
SRC = $(wildcard arch/mppa/*.c) \
      $(wildcard ipc/*.c)

# Object Files
OBJ_K1BDP = $(SRC:.c=.node.o)
OBJ_K1BIO = $(SRC:.c=.ioddr.o)

#===============================================================================

# Builds everything.
all: kernel-k1bdp kernel-k1bio

# Builds the kernel (node).
kernel-k1bdp: $(OBJ_K1BDP)
	mkdir -p $(LIBDIR)
	$(AR) $(ARFLAGS) $(LIBKERNEL_K1BDP) $(OBJ_K1BDP)

# Builds the kernel (ioddr).
kernel-k1bio: $(OBJ_K1BIO)
	mkdir -p $(LIBDIR)
	$(AR) $(ARFLAGS) $(LIBKERNEL_K1BIO) $(OBJ_K1BIO)

# Builds object files.
%.ioddr.o: %.c
	$(CC) -march=k1b -mos=rtems -mcluster=ioddr $(CFLAGS) $<  -c -o $@

# Builds object files.
%.node.o: %.c
	$(CC) -march=k1b -mos=nodeos -mcluster=node $(CFLAGS) $<  -c -o $@

# Cleans compilation files.
clean:
	rm -rf $(OBJ_K1BDP)
	rm -rf $(OBJ_K1BIO)
	rm -rf $(LIBKERNEL_K1BDP)
	rm -rf $(LIBKERNEL_K1BIO)
