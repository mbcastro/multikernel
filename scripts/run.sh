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

export K1TOOLS_DIR="/usr/local/k1tools"
export OUTDIR=.

# Global parameters.
NCLUSTERS=16
NITERATIONS=5
BUFSIZE=1048576

#
# Runs a multibinary file using a single IO Cluster.
#
function run1
{
	local multibin=$1
	local bin=$2
	local args=$3

	$K1TOOLS_DIR/bin/k1-jtag-runner     \
		--multibinary=$OUTDIR/$multibin \
		--exec-multibin=IODDR0:$bin     \
		-- $args

	local ret=$?
	if [ $ret == "0" ]
	then
		printf "[nanvix][test] %-30s \e[32m%s\e[0m\n" "$multibin" "passed"
	else
		printf "[nanvix][test] %-30s \e[91m%s\e[0m\n" "$multibin" "FAILED"
	fi
}

#
# Runs a multibinary file using two IO Clusters.
#
function run2
{
	local multibin=$1
	local bin1=$2
	local bin2=$3
	local args=$4

	$K1TOOLS_DIR/bin/k1-jtag-runner     \
		--multibinary=$OUTDIR/$multibin \
		--exec-multibin=IODDR0:$bin1    \
		--exec-multibin=IODDR1:$bin2    \
		-- $args

	local ret=$?
	if [ $ret == "0" ]
	then
		printf "[nanvix][test] %-30s \e[32m%s\e[0m\n" "$multibin" "passed"
	else
		printf "[nanvix][test] %-30s \e[91m%s\e[0m\n" "$multibin" "FAILED"
	fi
}


if [[ $1 == "test" ]];
then
	echo "Testing HAL"
	run2 "nanvix-debug.img" "/servers" "/servers1" "--debug --hal-core" | grep nanvix
	echo "Testing HAL Sync"
	run2 "nanvix-debug-hal-sync.img" "/servers" "/servers1" "--debug --hal-sync" | grep nanvix
	echo "Testing HAL Mailbox"
	run2 "nanvix-debug.img" "/servers" "/servers1" "--debug --hal-mailbox" | grep nanvix
	echo "Testing HAL Portal"
	run2 "nanvix-debug.img" "/servers" "/servers1" "--debug --hal-portal" | grep nanvix
	echo "Testing Naming Service"
	run2 "nanvix-debug.img" "/servers" "/servers1" "--debug --name" | grep nanvix
	echo "Testing mppa_waitpid()"
	run1 "waitpid.img" "/test/waitpid-master"
#	echo "Testing Mailbox"
#	run2 "nanvix-debug-mailbox.img" "/servers" "/servers1" "--debug --mailbox"
#	echo "Testing BARRIER"
#	run2 "test-barrier.img" "/test/barrier-master0" "/test/barrier-master1" "$NCLUSTERS" | grep "test"
#	echo "Testing RMEM"
elif [[ $1 == "benchmark" ]];
then
	case "$2" in
		mppa256-portal)
			for kernel in gather broadcast pingpong;
			do
				run1                                          \
					"benchmark-mppa256-portal.img"            \
					"/benchmark/mppa256-portal-master"        \
					"$NCLUSTERS $NITERATIONS $BUFSIZE $kernel"
			done
		;;
		nanvix-portal)
			for kernel in gather broadcast pingpong;
			do
				run1 "benchmark-hal-portal.img"                \
					"/benchmark/hal-portal-master"             \
					"$NCLUSTERS $NITERATIONS $BUFSIZE $kernel"
			done
		;;
		*)
			echo "Usage: run.sh test {mppa256-portal|nanvix-portal}"
			exit 1
		;;
	esac
fi
