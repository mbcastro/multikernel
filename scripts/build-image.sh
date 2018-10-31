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

BINDIR=$1

if [ -z $TARGET ]; then
	echo "$0: missing target architecture"
	exit 1
fi

if [ $TARGET == "mppa256" ]; then
	source "scripts/arch/mppa256.sh"
elif [ $TARGET == "unix" ]; then
	source "scripts/arch/unix.sh"
else
	echo "unkown architecture"
	exit 1
fi

#
# Missing arguments.
#
if [ -z $BINDIR ] ;
then
	echo "missing argument: binary directory"
	exit 1
fi

# Test Driver
BINARIES="$BINDIR/test/hal-sync-slave"
BINARIES="$BINARIES,$BINDIR/test/hal-mailbox-slave"
BINARIES="$BINARIES,$BINDIR/test/hal-portal-slave"
build2 $BINDIR $BINDIR/test-driver "$BINARIES" nanvix-kernel-debug.img

BINARIES="$BINDIR/test/ipc-name-slave"
BINARIES="$BINARIES,$BINDIR/test/ipc-barrier-slave"
BINARIES="$BINARIES,$BINDIR/test/ipc-mailbox-slave"
BINARIES="$BINARIES,$BINDIR/test/ipc-portal-slave"
BINARIES="$BINARIES,$BINDIR/test/mm-rmem-slave"
build2 $BINDIR $BINDIR/test-driver "$BINARIES" nanvix-runtime-debug.img

BINARIES="$BINDIR/test/posix-semaphore-slave"
build2 $BINDIR $BINDIR/test-driver "$BINARIES" nanvix-posix-debug.img
