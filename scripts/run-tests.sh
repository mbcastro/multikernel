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

source "scripts/arch/mppa256.sh"

# Command Parameters
test=$1 # Target test.
mode=$2 # Test mode.

# Missing parameters.
if [ -z $test ];
then
	echo "Missing arguments!"
	echo "Usage: $0 <test name> [mode]"
	exit 1 
fi

#
# Long test mode.
#
if [ -z $mode ];
then
	mode="--long"
fi

#
# Stops regression test if running running short test.
#
function stop_if_short_test
{
	if [ $mode == "--short" ];
	then
		exit 0
	fi
}

case $test in
	all)
		mode="--long"
	;&
	mqueue)
		echo "=== Running Message Queue Tests"
		run2 "nanvix-posix-debug.img" "/test-driver" "--debug --mqueue"
		stop_if_short_test
	;&
	shm)
		echo "=== Running Shared Memory Region Tests"
		run2 "nanvix-posix-debug.img" "/test-driver" "--debug --shm"
		stop_if_short_test
	;&
	rmem)
		echo "=== Running RMem Tests"
		run2 "nanvix-runtime-debug.img" "/test-driver" "--debug --rmem"
		stop_if_short_test
	;&
	semaphore)
		echo "=== Running Semaphore Tests"
		run2 "nanvix-posix-debug.img" "/test-driver" "--debug --semaphore"
		stop_if_short_test
	;&
	portal)
		echo "=== Running Nammed Portal Tests"
		run2 "nanvix-runtime-debug.img" "/test-driver" "--debug --portal"
		stop_if_short_test
	;&
	mailbox)
		echo "=== Running Nammed Mailbox Tests"
		run2 "nanvix-runtime-debug.img" "/test-driver" "--debug --mailbox"
		stop_if_short_test
	;&
	barrier)
		echo "=== Running Barrier Tests"
		run2 "nanvix-runtime-debug.img" "/test-driver" "--debug --barrier"
		stop_if_short_test
	;&
	name)
		echo "=== Running Naming Service Tests"
		run2 "nanvix-runtime-debug.img" "/test-driver" "--debug --name"
		stop_if_short_test
	;&
	kernel-portal)
		echo "=== Running Unnamed Portal Tests"
		run2 "nanvix-kernel-debug.img" "/test-driver" "--debug --hal-portal"
		stop_if_short_test
	;&
	kernel-mailbox)
		echo "=== Running Unnamed Mailbox Tests"
		run2 "nanvix-kernel-debug.img" "/test-driver" "--debug --hal-mailbox"
		stop_if_short_test $mode
	;&
	kernel-sync)
		echo "=== Running Unnamed Sync Tests"
		run2 "nanvix-kernel-debug.img" "/test-driver" "--debug --hal-sync"
		stop_if_short_test $mode
	;&
	kernel-core)
		echo "=== Running Core and NoC Interface Tests"
		run2 "nanvix-kernel-debug.img" "/test-driver" "--debug --hal-core"
		stop_if_short_test $mode
	;&
esac
