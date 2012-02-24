#!/bin/sh

# BLOCKSIZE varies from 16 to 64
# L1_SIZE varies from 1KB to 16KB
# L2 is 512KB. 
# L1_PREF_N and L1_PREF_M both vary from 1 to 4.
# Total 240 points. 15 sets of 16 points each.

tracefile="traces/gcc_trace.txt"

BLOCKSIZE=0

L1_SIZE=0
L1_ASSOC=4
L1_PREF_N=0
L1_PREF_M=0

L2_SIZE=65536
L2_ASSOC=8
L2_PREF_N=0
L2_PREF_M=0

rm -f points.6
echo "All factors vary! " > points.6

for (( j=4 ; j<=6 ; j++ )) ; do
	let BLOCKSIZE="1<<$j"
	for (( i=10 ; i<=14 ; i++ )) ; do
		let L1_SIZE="1<<$i"
		echo "$BLOCKSIZE $L1_SIZE" >> points.6
		for (( n=1 ; n<=4 ; n++)) ; do
			L1_PREF_N=$n
			for (( m=1 ; m<=4 ; m++ )) ; do
				L1_PREF_M=$m
				rm -f output.$i.$n.$m
				./sim_cache $BLOCKSIZE $L1_SIZE $L1_ASSOC $L1_PREF_N $L1_PREF_M $L2_SIZE $L2_ASSOC $L2_PREF_N $L2_PREF_M $tracefile > output.$i.$n.$m
				op=`cat output.$i.$n.$m | grep "L1 reads"`
				L1_reads=`echo $op | awk '{print $6}'`
				op=`cat output.$i.$n.$m | grep "L1 writes"`
				L1_writes=`echo $op | awk '{print $6}'`
				op=`cat output.$i.$n.$m | grep "L1 read misses"`
				L1_read_misses=`echo $op | awk '{print $7}'`
				op=`cat output.$i.$n.$m | grep "L1 write misses"`
				L1_write_misses=`echo $op | awk '{print $7}'`
				op=`cat output.$i.$n.$m | grep "L2 reads that did not originate from L1 prefetches"`
				L2_reads_not_L1_prefetch=`echo $op | awk '{print $13}'`
				op=`cat output.$i.$n.$m | grep "L2 read misses that did not originate from L1 prefetches"`
				L2_read_misses_not_L1_prefetch=`echo $op | awk '{print $14}'`
				echo "$i $L1_reads $L1_writes $L1_read_misses $L1_write_misses $L2_reads_not_L1_prefetch $L2_read_misses_not_L1_prefetch" >> points.6
				rm -f output.$i.$n.$m
			done
		done
	done
done

echo "The (X,Y) co-ordinates will be found in file points.6"

