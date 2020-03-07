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
all: all-qsort all-sha1

# Cleans up build objects.
clean: clean-qsort clean-sha1

# Cleans up everything.
distclean: distclean-qsort distclean-sha1

#===============================================================================
# Quick Sort Benchmarks
#===============================================================================

# Builds Quick Sort.
all-qsort:
	$(MAKE) -C qsort all

# Cleans up build objects.
clean-qsort:
	$(MAKE) -C qsort clean

# Cleans up everything from Quick Sort.
distclean-qsort:
	$(MAKE) -C qsort distclean

#===============================================================================
# SHA-1
#===============================================================================

# Builds SHA-1.
all-sha1:
	$(MAKE) -C sha1 all

# Cleans up build objects.
clean-sha1:
	$(MAKE) -C sha1 clean

# Cleans up everything
distclean-sha1:
	$(MAKE) -C sha1 distclean
