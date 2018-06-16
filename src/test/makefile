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
.PHONY: hal
.PHONY: hal-sync
.PHONY: hal-mailbox
.PHONY: hal-portal
.PHONY: name

# Change binaries directory.
export BINDIR := $(BINDIR)/test

# Builds everything.
all: hal hal-sync hal-mailbox hal-portal name

# Builds HAL testing unit. 
hal:
	mkdir -p $(BINDIR)
	cd hal && $(MAKE) all

# Builds HAL Sync testing unit. 
hal-sync:
	mkdir -p $(BINDIR)
	cd hal-sync && $(MAKE) all

# Builds HAL Mailbox testing unit. 
hal-mailbox:
	mkdir -p $(BINDIR)
	cd hal-mailbox && $(MAKE) all

# Builds HAL Portal testing unit. 
hal-portal:
	mkdir -p $(BINDIR)
	cd hal-portal && $(MAKE) all

# Builds Naming Service testing unit
name:
	mkdir -p $(BINDIR)
	cd name && $(MAKE) all

# Cleans compilation files.
clean:
	cd hal && $(MAKE) clean
	cd hal-sync && $(MAKE) clean
	cd hal-mailbox && $(MAKE) clean
	cd hal-portal && $(MAKE) clean
	cd name && $(MAKE) clean

# Cleans everything.
distclean:
	cd hal && $(MAKE) distclean
	cd hal-sync && $(MAKE) distclean
	cd hal-mailbox && $(MAKE) distclean
	cd hal-portal && $(MAKE) distclean
	cd name && $(MAKE) distclean

