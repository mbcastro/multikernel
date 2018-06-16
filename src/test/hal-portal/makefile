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

# Conflicts.
.PHONY: master
.PHONY: slave

# Binary Files
export ELFBIN_MASTER = $(BINDIR)/hal-portal-master
ifeq ($(TARGET),k1bio)
	MULTIBIN = $(BINDIR)/hal-portal.img
endif

#===============================================================================

# Builds binaries.
all: master

# Builds master binary.
master:
ifeq ($(TARGET),k1bio)
	cd master && $(MAKE) all
endif

# Builds image.
image: $(ELFBIN_MASTER)
ifeq ($(TARGET),k1bio)
	$(TOOLCHAIN)/bin/k1-create-multibinary -f --remove-prefix $(BINDIR) --boot $(ELFBIN_MASTER) -T $(MULTIBIN)
endif

# Cleans compilation files.
clean:
	cd master && $(MAKE) clean

# Cleans everything.
distclean: clean
	rm -rf $(MULTIBIN)
	cd master && $(MAKE) distclean

