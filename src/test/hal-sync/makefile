#
# MIT License
#
# Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
# "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
# LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

# Conflicts.
.PHONY: master0
.PHONY: master1
.PHONY: slave

#===============================================================================

# Builds binaries.
all: master0 master1 slave

# Builds master binary.
master0:
ifeq ($(BUILD_SERVERS),true)
	cd master0 && $(MAKE) all
endif

# Builds master binary.
master1:
ifeq ($(BUILD_SERVERS),true)
	cd master1 && $(MAKE) all
endif

# Builds slave binary.
slave:
ifeq ($(BUILD_SERVERS),false)
	cd slave && $(MAKE) all
endif

# Cleans compilation files.
clean:
	cd master0 && $(MAKE) clean
	cd master1 && $(MAKE) clean
	cd slave && $(MAKE) clean

# Cleans everything.
distclean: clean
	cd master0 && $(MAKE) distclean
	cd master1 && $(MAKE) distclean
	cd slave && $(MAKE) clean
