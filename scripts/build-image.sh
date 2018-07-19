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
TOOLCHAIN=/usr/local/k1tools

#
# Missing arguments.
#
if [ -z $BINDIR ] ;
then
	echo "missing argument: binary directory"
	exit 1
fi

function build0
{
	bootbin=$1
	nodebin=$2
	multibin=$3

	$TOOLCHAIN/bin/k1-create-multibinary -f \
		--remove-prefix $BINDIR             \
		--boot $bootbin                     \
		--clusters $nodebin                 \
		-T $multibin
}

function build1
{
	bootbin=$1
	iobin=$2
	multibin=$3

	$TOOLCHAIN/bin/k1-create-multibinary -f \
		--remove-prefix $BINDIR             \
		--boot $bootbin                     \
		--ios $iobin                        \
		-T $multibin
}

function build2
{
	bootbin=$1
	iobin=$2
	nodebin=$3
	multibin=$4

	$TOOLCHAIN/bin/k1-create-multibinary -f \
		--remove-prefix $BINDIR             \
		--boot $bootbin                     \
		--ios $iobin                        \
		--clusters $nodebin                 \
		-T $multibin
}

# Test Driver
BINARIES="$BINDIR/test/hal-sync-slave"
BINARIES="$BINARIES,$BINDIR/test/hal-mailbox-slave"
BINARIES="$BINARIES,$BINDIR/test/hal-portal-slave"
build2 $BINDIR/test-driver $BINDIR/servers1 "$BINARIES" nanvix-kernel-debug.img

BINARIES="$BINDIR/test/ipc-name-slave"
BINARIES="$BINARIES,$BINDIR/test/barrier-slave"
BINARIES="$BINARIES,$BINDIR/test/mailbox-slave"
BINARIES="$BINARIES,$BINDIR/test/semaphore-slave"
build2 $BINDIR/test-driver $BINDIR/servers1 "$BINARIES" nanvix-runtime-debug.img

# Benchmarks
build0 $BINDIR/benchmark/mppa256-portal-master $BINDIR/benchmark/mppa256-portal-slave benchmark-mppa256-portal.img
build0 $BINDIR/benchmark/mppa256-rqueue-master $BINDIR/benchmark/mppa256-rqueue-slave benchmark-mppa256-rqueue.img
build0 $BINDIR/benchmark/hal-sync-master $BINDIR/benchmark/hal-sync-slave benchmark-hal-sync.img
build0 $BINDIR/benchmark/hal-mailbox-master $BINDIR/benchmark/hal-mailbox-slave benchmark-hal-mailbox.img
build0 $BINDIR/benchmark/hal-portal-master $BINDIR/benchmark/hal-portal-slave benchmark-hal-portal.img
