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

	tar -cjvf benchmark-hal-sync.tar.bz2 *.benchmark
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
#	echo "Testing Mailbox"
#	run2 "nanvix-debug-mailbox.img" "/servers" "/servers1" "--debug --mailbox"
#	echo "Testing BARRIER"
#	run2 "test-barrier.img" "/test/barrier-master0" "/test/barrier-master1" "$NCLUSTERS" | grep "test"
#	echo "Testing RMEM"
#	run2 "rmem.img" "rmem-master" "rmem-server" "write $NCLUSTERS $SIZE"
#	run2 "rmem.img" "rmem-master" "rmem-server" "read $NCLUSTERS $SIZE"
elif [[ $1 == "benchmark" ]];
then
	if [[ $2 == "mailbox" ]];
	then
		benchmark_mailbox
	elif [[ $2 == "async" ]];
	then
		echo "Testing ASYNC"
		run1 "async.img" "master.elf" "$NCLUSTERS $SIZE"
	elif [[ $2 == "km" ]];
	then
		echo "Running KM PORTAL"
		run1 "km-portal.img" "km-portal-master" "--nclusters $NCLUSTERS --class $CLASS"
		echo "Running KM RMEM"
		run2 "km-rmem.img" "km-rmem-master" "rmem-server" "--nclusters $NCLUSTERS --class $CLASS"
	elif [[ $2 == "gf" ]];
	then
		echo "Running GF PORTAL"
		run1 "gf-portal.img" "gf-portal-master" "--nclusters $NCLUSTERS --class $CLASS"
		echo "Running GF RMEM"
		run2 "gf-rmem.img" "gf-rmem-master" "rmem-server" "--nclusters $NCLUSTERS --class $CLASS"
		echo "Running GF RMEM DENSE"
		run2 "gf-dense-rmem.img" "gf-dense-rmem-master" "rmem-server" "--nclusters $NCLUSTERS --class $CLASS"
	elif [[ $2 == "is" ]];
	then

		echo "Running IS PORTAL"
		run1 "is-portal.img" "is-portal-master" "--nclusters $NCLUSTERS --class $CLASS"
		echo "Running IS RMEM"
		run2 "is-rmem.img" "is-rmem-master" "rmem-server" "--nclusters $NCLUSTERS --class $CLASS"
	fi
fi
