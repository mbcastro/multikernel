#
# Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
#
# This file is part of Nanvix.
#
# Nanvix is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# Nanvix is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
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

build1 $BINDIR/servers $BINDIR/test/hal-master         test-hal.img
build1 $BINDIR/servers $BINDIR/test/hal-sync-master    test-hal-sync.img
build1 $BINDIR/servers $BINDIR/test/hal-mailbox-master test-hal-mailbox.img
build1 $BINDIR/servers $BINDIR/test/hal-portal-master  test-hal-portal.img

build2 $BINDIR/servers $BINDIR/test/name-master        $BINDIR/test/name-slave test-name.img

