#!/bin/sh

# CACHE_SIZE is varied, ASSOC is varied, BLOCKSIZE is fixed.
# L2 Cache: 512kB, 8 Way Set Associative ; Prefetching: None

# Y-axis: AAT ; X-axis: log2(CACHE_SIZE)
# CACHE_SIZE will be varied from 1kB to 256kB in powers of two. So X-axis values will be 
# 10, 11, 12, 13, ...., 18
# The above 9 runs will be repeated for Direct-Mapped, 2-way set associative, 4-way set-associative, 
# 8-way set-associative and fully-associative. 
# So total 45 runs. 5 lines on the graph - all 9 corresponding to a particular associativity will be a single curve.

tracefile="traces/gcc_trace.txt"

# Direct-Mapped, 1kB to 1MB L1 Cache-Sizes, No L2, No Prefetch 
BLOCKSIZE=32

L1_SIZE=0
L1_ASSOC=1
L1_PREF_N=0
L1_PREF_M=0

L2_SIZE=524288
L2_ASSOC=8
L2_PREF_N=0
L2_PREF_M=0
rm -f points.3
echo "(A) Direct-Mapped, L1 size 1kB to 256kB, No L1 Prefetch, L2 512kB, 8-way set associative, No L2 Prefetch, BlockSize 32" > points.3

for (( i=10; i<=18; i++)) ; do
	L1_SIZE=1
	let L1_SIZE="$L1_SIZE<<$i"
	# echo $i $L1_SIZE
	rm -f output.$i
	./sim_cache $BLOCKSIZE $L1_SIZE $L1_ASSOC $L1_PREF_N $L1_PREF_M $L2_SIZE $L2_ASSOC $L2_PREF_N $L2_PREF_M $tracefile > output.$i
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
	echo "$i $L1_reads $L1_writes $L1_read_misses $L1_write_misses $L2_reads_not_L1_prefetch $L2_read_misses_not_L1_prefetch" >> points.3
	rm -f output.$i
done

# 2-Way Set Associative, 1kB to 1MB L1 Cache-Sizes, L2, No Prefetch 
BLOCKSIZE=32

L1_SIZE=0
L1_ASSOC=2
L1_PREF_N=0
L1_PREF_M=0

L2_SIZE=524288
L2_ASSOC=8
L2_PREF_N=0
L2_PREF_M=0
echo "(B) 2-Way Set Associative, L1 size 1kB to 256kB, No L1 Prefetch, L2 512kB, 8-way set associative, No L2 Prefetch, BlockSize 32" >> points.3

for (( i=10; i<=18; i++)) ; do
	L1_SIZE=1
	let L1_SIZE="$L1_SIZE<<$i"
	# echo $i $L1_SIZE
	rm -f output.$i
	./sim_cache $BLOCKSIZE $L1_SIZE $L1_ASSOC $L1_PREF_N $L1_PREF_M $L2_SIZE $L2_ASSOC $L2_PREF_N $L2_PREF_M $tracefile > output.$i
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
	echo "$i $L1_reads $L1_writes $L1_read_misses $L1_write_misses $L2_reads_not_L1_prefetch $L2_read_misses_not_L1_prefetch" >> points.3
	rm -f output.$i
done

# 4-Way Set Associative, 1kB to 1MB L1 Cache-Sizes, L2, No Prefetch 
BLOCKSIZE=32

L1_SIZE=0
L1_ASSOC=4
L1_PREF_N=0
L1_PREF_M=0

L2_SIZE=524288
L2_ASSOC=8
L2_PREF_N=0
L2_PREF_M=0
echo "(C) 4-Way Set Associative, L1 size 1kB to 256kB, No L1 Prefetch, L2 512kB, 8-way set associative, No L2 Prefetch, BlockSize 32" >> points.3

for (( i=10; i<=18; i++)) ; do
	L1_SIZE=1
	let L1_SIZE="$L1_SIZE<<$i"
	# echo $i $L1_SIZE
	rm -f output.$i
	./sim_cache $BLOCKSIZE $L1_SIZE $L1_ASSOC $L1_PREF_N $L1_PREF_M $L2_SIZE $L2_ASSOC $L2_PREF_N $L2_PREF_M $tracefile > output.$i
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
	echo "$i $L1_reads $L1_writes $L1_read_misses $L1_write_misses $L2_reads_not_L1_prefetch $L2_read_misses_not_L1_prefetch" >> points.3
	rm -f output.$i
done

# 8-Way Set Associative, 1kB to 1MB L1 Cache-Sizes, L2, No Prefetch 
BLOCKSIZE=32

L1_SIZE=0
L1_ASSOC=8
L1_PREF_N=0
L1_PREF_M=0

L2_SIZE=524288
L2_ASSOC=8
L2_PREF_N=0
L2_PREF_M=0
echo "(D) 8-Way Set Associative, L1 size 1kB to 256kB, No L1 Prefetch, L2 512kB 8-way set associative, No L2 Prefetch, BlockSize 32" >> points.3

for (( i=10; i<=18; i++)) ; do
	L1_SIZE=1
	let L1_SIZE="$L1_SIZE<<$i"
	# echo $i $L1_SIZE
	rm -f output.$i
	./sim_cache $BLOCKSIZE $L1_SIZE $L1_ASSOC $L1_PREF_N $L1_PREF_M $L2_SIZE $L2_ASSOC $L2_PREF_N $L2_PREF_M $tracefile > output.$i
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
	echo "$i $L1_reads $L1_writes $L1_read_misses $L1_write_misses $L2_reads_not_L1_prefetch $L2_read_misses_not_L1_prefetch" >> points.3
	rm -f output.$i
done

# Fully Associative, 1kB to 1MB L1 Cache-Sizes, No L2, No Prefetch 
BLOCKSIZE=32

L1_SIZE=0
L1_ASSOC=0
L1_PREF_N=0
L1_PREF_M=0

L2_SIZE=524288
L2_ASSOC=8
L2_PREF_N=0
L2_PREF_M=0
echo "(E) Fully Associative, L1 size 1kB to 256kB, No L1 Prefetch, L2 512kB 8-way set associative, No L2 Prefetch, BlockSize 32" >> points.3

for (( i=10; i<=18; i++)) ; do
	L1_SIZE=1
	let L1_SIZE="$L1_SIZE<<$i"
	# echo $i $L1_SIZE
	L1_ASSOC=`expr $L1_SIZE / $BLOCKSIZE`
	# echo $L1_ASSOC
	rm -f output.$i
	./sim_cache $BLOCKSIZE $L1_SIZE $L1_ASSOC $L1_PREF_N $L1_PREF_M $L2_SIZE $L2_ASSOC $L2_PREF_N $L2_PREF_M $tracefile > output.$i
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
	echo "$i $L1_reads $L1_writes $L1_read_misses $L1_write_misses $L2_reads_not_L1_prefetch $L2_read_misses_not_L1_prefetch" >> points.3
	rm -f output.$i
done

echo "The (X,Y) co-ordinates will be found in file points.3"

