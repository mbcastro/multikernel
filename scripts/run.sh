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

export K1TOOLS_DIR="/usr/local/k1tools"
export OUTDIR=bin

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
		printf "[test] %-30s \e[32m%s\e[0m\n" "$multibin" "passed"
	else
		printf "[test] %-30s \e[91m%s\e[0m\n" "$multibin" "FAILED"
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
		printf "[test] %-30s \e[32m%s\e[0m\n" "$multibin" "passed"
	else
		printf "[test] %-30s \e[91m%s\e[0m\n" "$multibin" "FAILED"
	fi
}

if [[ $1 == "test" ]];
then
	echo "Testing HAL"
	run1 "hal.img" "/hal-master" | grep "test"
#	echo "Testing MAILBOX"
#	run1 "hal-mailbox.img" "hal-mailbox-master" | grep "test"
#	echo "Testing SYNC"
#	run1 "hal-sync.img" "hal-sync-master" | grep "test"
#	echo "Testing NAME"
#	run2 "name.img" "spawner-server" "name-master" "$NCLUSTERS" | grep "test"
#	echo "Testing PORTAL"
#	run2 "portal.img" "name-server" "portal-master" "write $NCLUSTERS $SIZE"
#	echo "Testing RMEM"
#	run2 "rmem.img" "rmem-master" "rmem-server" "write $NCLUSTERS $SIZE"
#	run2 "rmem.img" "rmem-master" "rmem-server" "read $NCLUSTERS $SIZE"
elif [[ $1 == "benchmark" ]];
then
	if [[ $2 == "async" ]];
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
