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
	nclusters=16
	size=$((16*1024))

#	echo "Testing ASYNC"
#	run1 "async-latency.img" "master.elf" "$nclusters $size"
#	echo "Testing PORTAL"
#	run1 "portal-latency.img" "portal-latency-master" "write $nclusters $size"
#	echo "Testing Write RMEM"
#	run2 "rmem-latency.img" "rmem-latency-master" "rmem-server" "write $nclusters $size"
#	run2 "rmem-latency.img" "rmem-latency-master" "rmem-server" "read $nclusters $size"
	echo "Kmean Kernel"
	run1 "kmeans.img" "kmeans-master" "--nclusters $nclusters --class tiny --verbose"
else
	for nclusters in 4 8 12 16;
	do
		echo "Running $nclusters"
		for size in 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576;
		do
			run1 "async-latency.img" "master.elf" "$nclusters $size" >> async-latency.out
			run1 "portal-latency.img" "portal-latency-master" "$nclusters $size" >> portal-latency.out
			run2 "rmem-latency.img" "rmem-latency-master" "rmem-server" "$nclusters $size" >> rmem-latency.out
		done
	done
fi
