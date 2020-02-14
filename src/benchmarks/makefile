#
# MIT License
#
# Copyright(c) 2011-2019 The Maintainers of Nanvix
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# Builds everything.
all: all-rcache all-vmem

# Cleans up build objects.
clean: clean-rcache clean-vmem

# Cleans up everything.
distclean: distclean-rcache distclean-vmem

#===============================================================================
# RMem Cache Benchmarks
#===============================================================================

# Builds RMem Cache benchmarks.
all-rcache:
	$(MAKE) -C rcache all

# Cleans up build objects of RMem Cache benchmarks.
clean-rcache:
	$(MAKE) -C rcache clean

# Cleans up everything from RMem Cache benchmarks.
distclean-rcache:
	$(MAKE) -C rcache distclean

#===============================================================================
# VMem Allocator Benchmarks
#===============================================================================

# Builds VMem Allocator benchmarks.
all-vmem:
	$(MAKE) -C vmem all

# Cleans up build objects of VMem Allocator benchmarks.
clean-vmem:
	$(MAKE) -C vmem clean

# Cleans up everything from VMem Allocator benchmarks.
distclean-vmem:
	$(MAKE) -C vmem distclean
