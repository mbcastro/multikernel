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

# Global parameters.
NCLUSTERS=16
NITERATIONS=5
BUFSIZE=1048576

case "$1" in
	mppa256-rqueue)
		echo "Running MPPA-256 Rqueue Microbenchmarks"
		for kernel in broadcast;
		do
			run1                                          \
				"benchmark-mppa256-rqueue.img"            \
				"/benchmark/mppa256-rqueue-master"        \
				"$NCLUSTERS $NITERATIONS $kernel"
		done
	;;
	mppa256-portal)
		echo "Running MPPA-256 Portal Microbenchmarks"
		for kernel in gather broadcast pingpong;
		do
			run1                                          \
				"benchmark-mppa256-portal.img"            \
				"/benchmark/mppa256-portal-master"        \
				"$NCLUSTERS $NITERATIONS $BUFSIZE $kernel"
		done
	;;
	nanvix-mailbox)
		echo "Running Nanvix Mailbox Microbenchmarks"
		for kernel in gather broadcast pingpong;
		do
			run1 "benchmark-hal-mailbox.img"      \
				"/benchmark/hal-mailbox-master"   \
				"$NCLUSTERS $NITERATIONS $kernel"
		done
	;;
	nanvix-portal)
		echo "Running Nanvix Portal Microbenchmarks"
		for kernel in gather broadcast pingpong;
		do
			run1 "benchmark-hal-portal.img"                \
				"/benchmark/hal-portal-master"             \
				"$NCLUSTERS $NITERATIONS $BUFSIZE $kernel"
		done
	;;
	*)
		echo "Usage: run.sh test {mppa256-portal|mppa256-rqueue|nanvix-portal}"
		exit 1
	;;
esac

