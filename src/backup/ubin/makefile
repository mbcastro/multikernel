#
# MIT License
#
# Copyright (c) 211-218 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
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
.PHONY: test

export CFLAGS += -I $(INCDIR)

#
# Fix conflicting symbols.
#
export CFLAGS +=                     \
	-D sem_close=nanvix2_sem_close   \
	-D sem_open=nanvix2_sem_open     \
	-D sem_post=nanvix2_sem_post     \
	-D sem_unlink=nanvix2_sem_unlink \
	-D sem_wait=nanvix2_sem_wait     \
	-D mmap=nanvix2_mmap             \
	-D munmap=nanvix2_munmap         \
	-D shm_open=nanvix2_shm_open     \
	-D shm_unlink=nanvix2_shm_unlink \
	-D ftruncate=nanvix2_ftruncate

#===============================================================================

# Builds everything.
all: test

# Builds Test-Driver Utility.
test:
	cd test && $(MAKE) all

# Cleans compilation files.
clean:
	cd test && $(MAKE) clean

# Cleans everything.
distclean: clean
	cd test && $(MAKE) distclean
