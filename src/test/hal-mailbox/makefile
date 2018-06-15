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
SRC = $(wildcard *.c)

# Object Files
OBJ = $(SRC:.c=.o)

# Binary Files
ELFBIN = $(BINDIR)/hal-mailbox-master
MULTIBIN = $(BINDIR)/hal-mailbox.img

#===============================================================================

# Builds binaries.
all: $(OBJ)
	mkdir -p $(BINDIR)
ifeq ($(TARGET),k1bio)
	$(CC) $(LDFLAGS) $(OBJ) -o $(ELFBIN) $(LIBKERNEL) $(LIBMPPAIPC)
endif

# Builds image.
image: $(ELFBIN)
	$(TOOLCHAIN)/bin/k1-create-multibinary -f --remove-prefix $(BINDIR) --boot $(ELFBIN) -T $(MULTIBIN)

# Builds object files.
%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

# Cleans compilation files.
clean:
	rm -rf $(OBJ)
#	rm -rf $(ELFBIN)
#	rm -rf $(MULTIBIN)

