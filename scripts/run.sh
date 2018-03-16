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
# Runs a unit test.

# $1 Test unit name.
#
function run_test
{
	$K1TOOLS_DIR/bin/k1-jtag-runner             \
		--multibinary=$OUTDIR/test.img          \
		--exec-multibin=IODDR0:master.test      \
		--exec-multibin=IODDR1:rmem-server.test \
		-- $1
}

for ncclusters in 1 2 4 8 16;
do
	echo "=== TESTING $ncclusters"
	for pattern in regular irregular;
	do
		for workload in write read mixed;
		do
			run_test "rmem --ncclusters $ncclusters --pattern $pattern --workload $workload"
			echo "---"
		done
	done
done
