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
export ELFBIN_MASTER = $(BINDIR)/name-master
export ELFBIN_SLAVE = $(BINDIR)/name-slave
export MULTIBIN = $(BINDIR)/name.img

all: master slave
	$(TOOLCHAIN)/bin/k1-create-multibinary --remove-prefix $(BINDIR) --boot $(SERVERSBIN) --ios $(ELFBIN_MASTER)  --clusters $(ELFBIN_SLAVE) -T $(MULTIBIN)

# Builds master binary.
master:
	cd master && $(MAKE) all

# Builds slave binary.
slave:
	cd slave && $(MAKE) all

# Cleans compilation files.
clean:
	cd master && $(MAKE) clean

