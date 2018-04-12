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
export OUTDIR=output/bin/

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
}

if [ $1 == "test" ];
then
	nclusters=1
	size=$((16*1024))
	nmessages=2

	echo "Testing ASYNC"
	run1 "async.img" "master.elf" "$nclusters $size"
	echo "Testing PORTAL"
	run1 "portal.img" "portal-master" "write $nclusters $size"
	echo "Testing MAILBOX"
	run1 "mailbox.img" "mailbox-master" "$nclusters $nmessages"
	echo "Testing RMEM"
	run2 "rmem.img" "rmem-master" "rmem-server" "write $nclusters $size"
	run2 "rmem.img" "rmem-master" "rmem-server" "read $nclusters $size"
	echo "Testing Gaussian Filter with Portal"
	run1 "gf-portal.img" "gf-portal-master" "--nclusters $nclusters --class tiny --verbose"
else
	for nclusters in 4 8 12 16;
	do
		echo "Running IS"
		run1 "is.img" "is-master" "--nclusters $nclusters --class tiny"
		echo "Running KM"
		run1 "km-portal.img" "km-portal-master"            "--nclusters $nclusters --class tiny"
		run2 "km-rmem.img" "km-rmem-master" "rmem-server" "--nclusters $nclusters --class tiny"	
	done
fi
