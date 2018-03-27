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
# Runs a multibinary file.
#
function run
{
	local multibin=$1
	local bin=$2
	local args=$3

	$K1TOOLS_DIR/bin/k1-jtag-runner     \
		--multibinary=$OUTDIR/$multibin \
		--exec-multibin=IODDR0:$bin     \
		-- $args
}

if [ $1 == "test" ];
then
	nclusters=4
	size=$((1024*1024))

	run "portal-latency.img" "portal-latency-master" "$nclusters $size"
	run "async-latency.img" "master.elf" "$nclusters $size"
else
	for nclusters in 4 8 12 16;
	do
		echo "Running $nclusters"
		for size in 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576;
		do
			run "portal-latency.img" "portal-latency-master" "$nclusters $size" >> portal-latency.out
			run "async-latency.img" "master.elf" "$nclusters $size" >> async-latency.out
		done
	done
fi
