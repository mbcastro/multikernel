#
# MIT License
#
# Copyright(c) 2011-2020 The Maintainers of Nanvix
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
all: all-rcache all-vmem all-pgfault all-memalloc all-memread all-memwrite

# Cleans up build objects.
clean: clean-rcache clean-vmem clean-pgfault clean-memalloc clean-memread \
	clean-memwrite

# Cleans up everything.
distclean: distclean-rcache distclean-vmem distclean-pgfault \
	distclean-memalloc distclean-memread distclean-memwrite

#===============================================================================
# RMem Cache Benchmarks
#===============================================================================

# Builds RMem Cache benchmark.
all-rcache:
	$(MAKE) -C rcache all

# Cleans up build objects of RMem Cache benchmark.
clean-rcache:
	$(MAKE) -C rcache clean

# Cleans up everything from RMem Cache benchmark.
distclean-rcache:
	$(MAKE) -C rcache distclean

#===============================================================================
# VMem Allocator Benchmark
#===============================================================================

# Builds VMem Allocator benchmark.
all-vmem:
	$(MAKE) -C vmem all

# Cleans up build objects of VMem Allocator benchmark.
clean-vmem:
	$(MAKE) -C vmem clean

# Cleans up everything from VMem Allocator benchmark.
distclean-vmem:
	$(MAKE) -C vmem distclean

#===============================================================================
# Page Fault Benchmark
#===============================================================================

# Builds Page Fault benchmark.
all-pgfault:
	$(MAKE) -C pgfault all

# Cleans up build objects.
clean-pgfault:
	$(MAKE) -C pgfault clean

# Cleans up everything.
distclean-pgfault:
	$(MAKE) -C pgfault distclean

#===============================================================================
# Memory Allocation
#===============================================================================

# Builds Memory Allocation benchmark.
all-memalloc:
	$(MAKE) -C memalloc all

# Cleans up build object.
clean-memalloc:
	$(MAKE) -C memalloc clean

# Cleans up everything.
distclean-memalloc:
	$(MAKE) -C memalloc distclean

#===============================================================================
# Memory Read
#===============================================================================

# Builds Memory Read benchmark.
all-memread:
	$(MAKE) -C memread all

# Cleans up build object.
clean-memread:
	$(MAKE) -C memread clean

# Cleans up everything.
distclean-memread:
	$(MAKE) -C memread distclean

#===============================================================================
# Memory Write
#===============================================================================

# Builds Memory Write benchmark.
all-memwrite:
	$(MAKE) -C memwrite all

# Cleans up build object.
clean-memwrite:
	$(MAKE) -C memwrite clean

# Cleans up everything.
distclean-memwrite:
	$(MAKE) -C memwrite distclean
