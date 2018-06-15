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
.PHONY: kernel
.PHONY: servers
.PHONY: test

# Builds the kernel.
kernel:
	cd kernel && $(MAKE) all

# Builds system servers
servers:
	cd servers && $(MAKE) all

# Builds testing system.
test:
	cd test && $(MAKE) all

# Cleans compilation files.
clean:
	cd kernel && $(MAKE) clean
	cd servers && $(MAKE) clean
	cd test && $(MAKE) clean
