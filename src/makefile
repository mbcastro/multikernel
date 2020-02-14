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

# Builds Everything
all: all-libruntime all-sbin all-sys all-ubin all-benchmarks

# Cleans Build Objects
clean: clean-libruntime clean-sbin clean-sys clean-ubin clean-benchmarks

# Cleans Everything
distclean: distclean-libruntime distclean-sbin distclean-sys distclean-ubin \
	distclean-benchmarks

#===============================================================================
# Runtime Library
#===============================================================================

# Builds Runtime Library.
all-libruntime:
	$(MAKE) -C libruntime all

# Cleans Run Time Library Build objects.
clean-libruntime:
	$(MAKE) -C libruntime clean

# Cleans Run Time Library build.
distclean-libruntime:
	$(MAKE) -C libruntime distclean

#===============================================================================
# Superuser Binaries
#===============================================================================

# Builds Superuser Binaries.
all-sbin: all-libruntime all-sys
	$(MAKE) -C sbin all

# Cleans Superuser Binaries Build objects.
clean-sbin:
	$(MAKE) -C sbin clean

# Cleans Superuser Binaries build.
distclean-sbin:
	$(MAKE) -C sbin distclean

#===============================================================================
# System Services
#===============================================================================

# Builds system services.
all-sys: all-libruntime
	$(MAKE) -C sys all

# cleans objects of system services.
clean-sys:
	$(MAKE) -C sys clean

# Cleans build of system services.
distclean-sys:
	$(MAKE) -C sys distclean

#===============================================================================
# User Binaries
#===============================================================================

# Builds User Binaries.
all-ubin: all-libruntime
	$(MAKE) -C ubin all

# Cleans User Binaries Build objects.
clean-ubin:
	$(MAKE) -C ubin clean

# Cleans User Binaries build.
distclean-ubin:
	$(MAKE) -C ubin distclean

#===============================================================================
# Benchmarks
#===============================================================================

# Builds User Binaries.
all-benchmarks: all-libruntime
	$(MAKE) -C benchmarks all

# Cleans User Binaries Build objects.
clean-benchmarks:
	$(MAKE) -C benchmarks clean

# Cleans User Binaries build.
distclean-benchmarks:
	$(MAKE) -C benchmarks distclean
