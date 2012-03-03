#!/bin/sh

# L1: L1_SIZE is varied, ASSOC is fixed (4), BLOCKSIZE is fixed (32).
# L2: L2_SIZE is varied, ASSOC is fixed (8), BLOCKSIZE is fixed (32).
# Prefetching: None

# Y-axis: AAT ; X-axis: log2(L1_SIZE)
# Vary L2_SIZE from 32KB, 64KB, 128KB, 256B, 512KB & 1MB. 
# For each of the L2_SIZE values, there will be one graph. 
# L1_SIZE is varied from 1KB to 256KB.

tracefile="traces/gcc_trace.txt"

BLOCKSIZE=32

L1_SIZE=0
L1_ASSOC=4
L1_PREF_N=0
L1_PREF_M=0

L2_SIZE=0
L2_ASSOC=8
L2_PREF_N=0
L2_PREF_M=0
rm -f points.5
echo "(A) L1 size varied from 1KB to 256KB, L2 size varied from 32KB to 1MB. No L1, L2 Prefetch, BlockSize 32" > points.5

for (( j=15 ; j<=20 ; j++)) ; do
	let L2_SIZE="1<<$j"
	echo "L2_SIZE=$L2_SIZE" >> points.5
	for (( i=10; i<=18; i++)) ; do
		L1_SIZE=1
		let L1_SIZE="$L1_SIZE<<$i"
		if [ "$L1_SIZE" -lt "$L2_SIZE" ] ; then 
			# echo $i $L1_SIZE
			rm -f output.$i
			./sim_cache $BLOCKSIZE $L1_SIZE $L1_ASSOC $L1_PREF_N $L1_PREF_M $L2_SIZE $L2_ASSOC $L2_PREF_N $L2_PREF_M $tracefile > output.$i
			op=`cat output.$i | grep "L1 miss rate"`
			L1_miss_rate=`echo $op | awk '{print $5}'`
			op=`cat output.$i | grep "L1 reads"`
			L1_reads=`echo $op | awk '{print $6}'`
			op=`cat output.$i | grep "L1 writes"`
			L1_writes=`echo $op | awk '{print $6}'`
			op=`cat output.$i | grep "L1 read misses"`
			L1_read_misses=`echo $op | awk '{print $7}'`
			op=`cat output.$i | grep "L1 write misses"`
			L1_write_misses=`echo $op | awk '{print $7}'`
			op=`cat output.$i | grep "L2 reads that did not originate from L1 prefetches"`
			L2_reads_not_L1_prefetch=`echo $op | awk '{print $13}'`
			op=`cat output.$i | grep "L2 read misses that did not originate from L1 prefetches"`
			L2_read_misses_not_L1_prefetch=`echo $op | awk '{print $14}'`
			echo "$i $L1_miss_rate $L1_reads $L1_writes $L1_read_misses $L1_write_misses $L2_reads_not_L1_prefetch $L2_read_misses_not_L1_prefetch" >> points.5
			rm -f output.$i
		fi
	done
done

echo "The (X,Y) co-ordinates will be found in file points.5"

