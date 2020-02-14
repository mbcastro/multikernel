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

export OUTDIR=.

#
# Runs a multibinary file using a single IO Cluster.
#
function run1
{
	local multibin=$1
	local bin=$2
	local args=$3

	xterm -e $PWD/bin/spawner0 $args &
	xterm -e $PWD/bin/spawner1 $args &

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
	local args=$3
	local bin2="/spawner1"

	if [ -z $DEBUG ];
	then
		local LAUNCHER='bash -c'
	else
		local LAUNCHER='xterm -e'
	fi;

	$LAUNCHER "$PWD/bin/spawner0 $args 2>&1 | tee spawner0.out; exit ${PIPESTATUS[0]}" &
	spawner0_pid=$!

	#
	# Force behaviour in which spawner0
	# runs in the first booting cluster.
	#
	sleep 1
	
	$LAUNCHER "$PWD/bin/spawner1 2>&1 | tee spawner1.out; exit ${PIPESTATUS[0]}" &
	spawner1_pid=$!

	wait $spawner1_pid
	spawner1_ret=$?

	wait $spawner0_pid
	spawner0_ret=$?

	if [ $spawner0_ret == "0" ] && [ $spawner1_ret == "0" ]
	then
		printf "[nanvix][test] %-30s \e[32m%s\e[0m\n" "$multibin" "passed"
	else
		printf "[nanvix][test] %-30s \e[91m%s\e[0m\n" "$multibin" "FAILED"
	fi
}

function build1
{
	local bindir=$1
	local bootbin=$2
	local nodebin=$3
	local multibin=$4

	echo "building binaries..."
}

function build2
{
	local bindir=$1
	local bootbin=$2
	local nodebin=$3
	local multibin=$4
	local iobin=$1/spawner1

	echo "building binaries..."
}

