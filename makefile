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

#===============================================================================
# Directories
#===============================================================================

# Directories.
export BINDIR  = $(CURDIR)/bin
export INCDIR  = $(CURDIR)/include
export LIBDIR  = $(CURDIR)/lib
export MAKEDIR = $(CURDIR)/build
export SRCDIR  = $(CURDIR)/src
export TOOLSDIR = $(CURDIR)/scripts

#===============================================================================

# Builds everything.
all: image

# Builds image.
image: nanvix
	bash $(TOOLSDIR)/build-image.sh $(BINDIR)

# Builds binaries.
nanvix:
ifeq ($(TARGET),mppa256)
	cd $(SRCDIR) && $(MAKE) all ARCH=k1bdp
	cd $(SRCDIR) && $(MAKE) all ARCH=k1bio
endif

# Cleans everything.
distclean:
	cd $(SRCDIR) && $(MAKE) distclean

