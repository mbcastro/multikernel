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

# Benchmark-specific parameters.
CLASS=small

# Testing unit specific parameters.
NMESSAGES=2
SIZE=$((16*1024))

#
# Runs a multibinary file in a single IO CLUSTER.
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
# Runs a multibinary file in the two IO Clusters.
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

#
# Benchmarks HAL Mailbox
#
function benchmark_mailbox
{
	echo "Benchmarking HAL Mailbox"

	echo "  nlocals=1 nremotes=1 (baseline)"
	run1 "benchmark-hal-mailbox.img" "/benchmark/hal-mailbox-master" "1 1 row 1" \
		| head -n -1                                                             \
		| cut -d" " -f 5                                                         \
		> hal-mailbox-1-1-row.benchmark

	for nlocals in 1 2 4;
	do
		for nremotes in 4 8 12 16;
		do
			echo "  nlocals=$nlocals nremotes=$nremotes"
			run1 "benchmark-hal-mailbox.img" "/benchmark/hal-mailbox-master" "$nlocals $nremotes row 4" \
				| head -n -1                                                                            \
				| cut -d" " -f 5                                                                        \
				> hal-mailbox-$nlocals-$nremotes-row.benchmark
			run1 "benchmark-hal-mailbox.img" "/benchmark/hal-mailbox-master" "$nlocals $nremotes col 4" \
				| head -n -1                                                                            \
				| cut -d" " -f 5                                                                        \
				> hal-mailbox-$nlocals-$nremotes-col.benchmark
		done
	done

	tar -cjvf benchmark-hal-mailbox.tar.bz2 *.benchmark
	rm -rf *.benchmark
}

#
# Benchmarks HAL Portal
#
function benchmark_portal
{
	echo "Benchmarking HAL Portal"

	let niterations=30

	for mode in "pingpong" "gather" "broadcast";
	do
		for bufsize in 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576
		do
			for nremotes in {1..16};
			do
				echo "  nremotes=$nremotes bufsize=$bufsize mode=$mode"

				run1 "benchmark-hal-portal.img"             \
					"/benchmark/hal-portal-master"          \
					"$nremotes $niterations $bufsize $mode" \
				| head -n -1                                \
				| cut -d" " -f 4                            \
				> hal-mailbox-$mode-$nremotes-$bufsize.benchmark
			done
		done
	done

	tar -cjvf benchmark-hal-portal.tar.bz2 *.benchmark
	rm -rf *.benchmark
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
#	run2 "rmem.img" "rmem-master" "rmem-server" "write $NCLUSTERS $SIZE"
#	run2 "rmem.img" "rmem-master" "rmem-server" "read $NCLUSTERS $SIZE"
elif [[ $1 == "benchmark" ]];
then
	case "$2" in
		mppa256portal)
			run1 "benchmark-mppa256-portal.img" \
			"/benchmark/mppa256-portal-master"  \
			"16 5 1048576 gather"
		;;
		mailbox)
			benchmark_mailbox
		;;
		portal)
			benchmark_portal
		;;
		*)
			echo "Usage: run.sh test {mppa256|mailbox|portal}"
			exit 1
		;;
	esac
fi
