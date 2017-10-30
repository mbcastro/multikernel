# Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

export BINDIR=$PWD/bin

trap 'kill $(jobs -p)' EXIT

#$BINDIR/ipc.test --server &
#sleep 1
#$BINDIR/ipc.test --client 

taskset -c 1 $BINDIR/ramdisk /tmp/ramdisk0 & sleep 1
taskset -c 2 $BINDIR/ramdisk /tmp/ramdisk1 & sleep 1
taskset -c 3 $BINDIR/ramdisk /tmp/ramdisk2 & sleep 1
taskset -c 4 $BINDIR/ramdisk /tmp/ramdisk3 & sleep 1
taskset -c 5 $BINDIR/bdev                  & sleep 1
taskset -c 6 $BINDIR/bdev.test 
