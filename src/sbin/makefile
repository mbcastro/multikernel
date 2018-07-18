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

# Conflicts
.PHONY: benchmarks
.PHONY: spawner0
.PHONY: spawner1

#===============================================================================

# Builds everything.
all: benchmarks spawner0 spawner1

# Builds benchmarks.
benchamrks:
	cd benchmarks && $(MAKE) all

# Builds spawner0 binaries.
spawner0:
	cd spawner0 && $(MAKE) all

# Builds spawner1 binaries.
spawner1:
ifeq ($(BUILD_SERVERS),true)
	cd spawner1 && $(MAKE) all
endif

# Cleans compilation files.
clean:
	cd benchmarks && $(MAKE) clean
	cd spawner0 && $(MAKE) clean
	cd spawner1 && $(MAKE) clean

# Cleans everything.
distclean: clean
	cd benchmarks && $(MAKE) distclean
	cd spawner0 && $(MAKE) distclean
	cd spawner1 && $(MAKE) distclean
