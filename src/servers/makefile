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

# C Source Files
SRC0 = spawner0.c                         \
       $(wildcard tests0/hal/core/*.c)    \
       $(wildcard tests0/hal/sync/*.c)    \
       $(wildcard tests0/hal/mailbox/*.c) \
       $(wildcard tests0/hal/portal/*.c)  \
       $(wildcard tests0/ipc/name/*.c)    \
       $(wildcard tests0/ipc/mailbox/*.c) \
       $(wildcard tests0/*.c)

# C Source Files
SRC1 = spawner1.c             \
	   name.c                 \
       $(wildcard tests1/*.c)

# Object Files
OBJ0 = $(SRC0:.c=.o)

# Object Files
OBJ1 = $(SRC1:.c=.o)

#===============================================================================

# Builds everything.
all: servers0 servers1

#
# Server fleet 0.
#
servers0: $(OBJ0)
	mkdir -p $(BINDIR)
	$(CC) $(LDFLAGS) $(OBJ0) -o $(SERVERSBIN0) $(LIBKERNEL) $(LIBS)

#
# Server fleet 1.
#
servers1: $(OBJ1)
	mkdir -p $(BINDIR)
	$(CC) $(LDFLAGS) $(OBJ1) -o $(SERVERSBIN1) $(LIBKERNEL) $(LIBS)

# Builds object files.
%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

# Cleans compilation files.
clean:
	rm -rf $(OBJ)

# Cleans everything.
distclean: clean
	rm -rf $(SERVERSBIN0)
	rm -rf $(SERVERSBIN1)
