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
all: all-libruntime all-zombie all-spawn all-name

# Cleans Build Objects
clean: clean-libruntime clean-zombie clean-spawn clean-name

# Cleans Everything
distclean: distclean-libruntime distclean-zombie distclean-spawn distclean-name

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
# Zombie Server
#===============================================================================

# Builds Zombie Server.
all-zombie: all-libruntime
	$(MAKE) -C zombie all

# Cleans Zombie Server Build objects.
clean-zombie:
	$(MAKE) -C zombie clean

# Cleans Zombie Server build.
distclean-zombie:
	$(MAKE) -C zombie distclean

#===============================================================================
# Spawn Server
#===============================================================================

# Builds Spawn Server.
all-spawn: all-libruntime
	$(MAKE) -C spawn all

# Cleans Spawn Server Build objects.
clean-spawn:
	$(MAKE) -C spawn clean

# Cleans Spawn Server build.
distclean-spawn:
	$(MAKE) -C spawn distclean

#===============================================================================
# Name Server
#===============================================================================

# Builds Name Server.
all-name: all-libruntime
	$(MAKE) -C name all

# Cleans Name Server Build objects.
clean-name:
	$(MAKE) -C name clean

# Cleans Name Server build.
distclean-name:
	$(MAKE) -C name distclean
