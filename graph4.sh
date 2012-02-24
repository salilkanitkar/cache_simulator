#!/bin/sh

# CACHE_SIZE is varied, ASSOC is fixed, BLOCKSIZE is varied.
# L2 Cache: None ; Prefetching: None

# Y-axis: L1 miss rate ; X-axis: log2(BLOCKSIZE)
# BLOCKSIZE will be varied as 16, 32, 64, 128. So X-axis values will be 4, 5, 6, 7.
# L1_SIZE will be varied from 1KB to 32KB, in powers of two.
# 6 different curves - each with 4 points. 
# So total 24 runs.

tracefile="traces/gcc_trace.txt"

BLOCKSIZE=0

L1_SIZE=0
L1_ASSOC=4
L1_PREF_N=0
L1_PREF_M=0

L2_SIZE=0
L2_ASSOC=0
L2_PREF_N=0
L2_PREF_M=0
rm -f points.4
echo "(A) 4-way set Associative, L1 size 1kB to 32kB, No L1 Prefetch, No L2, No L2 Prefetch, BlockSize 16 to 128" > points.4

for (( j=4 ; j<=7 ; j++)) ; do
	let BLOCKSIZE="1<<$j"
	echo "BlockSize=$BLOCKSIZE" >> points.4
	for (( i=10; i<=15; i++)) ; do
		L1_SIZE=1
		let L1_SIZE="$L1_SIZE<<$i"
		# echo $i $L1_SIZE
		rm -f output.$i
		./sim_cache $BLOCKSIZE $L1_SIZE $L1_ASSOC $L1_PREF_N $L1_PREF_M $L2_SIZE $L2_ASSOC $L2_PREF_N $L2_PREF_M $tracefile > output.$i
		op=`cat output.$i | grep "L1 miss rate"`
		L1_miss_rate=`echo $op | awk '{print $5}'`
		echo "$i $L1_miss_rate" >> points.4
		rm -f output.$i
	done
done

echo "The (X,Y) co-ordinates will be found in file points.4"

