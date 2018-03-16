for ncclusters in 1 2 4 8 16;
do
	for i in 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576;
	do
		file=results/noc-latency/noc-latency-1.out

		cat $file            \
			| grep "$i;"     \
			| tail -n 30     \
			| cut -d";" -f 2 \
		> /tmp/noc-latency-$i-$ncclusters.tmp
	done

	paste -d";"                      \
		/tmp/noc-latency-?-$ncclusters.tmp       \
		/tmp/noc-latency-??-$ncclusters.tmp      \
		/tmp/noc-latency-???-$ncclusters.tmp     \
		/tmp/noc-latency-????-$ncclusters.tmp    \
		/tmp/noc-latency-?????-$ncclusters.tmp   \
		/tmp/noc-latency-??????-$ncclusters.tmp  \
		/tmp/noc-latency-???????-$ncclusters.tmp \
	> results/noc-latency-$ncclusters.csv

	cat results/rmem/rmem-regular-write-$ncclusters.out \
		| grep -v "\[IODDR0\]"                          \
		| grep -v RMEM                                  \
		| cut -d" " -f 4-                               \
		| cut -d";" -f 2                                \
	> results/rmem-$ncclusters.csv
done

rm -f /tmp/*.tmp
