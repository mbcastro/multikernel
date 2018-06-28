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

build1 $BINDIR/servers $BINDIR/test/hal-master                           test-hal.img
build1 $BINDIR/test/hal-mailbox-master0 $BINDIR/test/hal-mailbox-master1 test-hal-mailbox.img
build1 $BINDIR/servers $BINDIR/test/hal-portal-master                    test-hal-portal.img

build2 $BINDIR/test/hal-sync-master0 $BINDIR/test/hal-sync-master1 $BINDIR/test/sync-slave         test-hal-sync.img
build2 $BINDIR/servers               $BINDIR/test/name-master      $BINDIR/test/name-slave         test-name.img
build2 $BINDIR/servers               $BINDIR/test/mailbox-master   $BINDIR/test/mailbox-slave      test-mailbox.img

build2 $BINDIR/servers $BINDIR/benchmark/hal-mailbox-master $BINDIR/benchmark/hal-mailbox-slave  benchmark-hal-mailbox.img
