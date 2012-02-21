#!/bin/sh

make clean ; make DEBUG_OP=1

# rm -f debug.op0 debug.op1 debug.op2 debug.op3
# rm -f diff.debug.op0 diff.debug.op1 diff.debug.op2 diff.debug.op3

# ./sim_cache 16 1024 2 0 0 0 0 0 0 traces/gcc_trace.txt > debug.op0
# diff -iw debug.op0 debug_runs/gcc.debug0.txt > diff.debug.op0
# echo "===================== gcc.debug0 ====================="
# cat diff.debug.op0
# rm -f debug.op0

# ./sim_cache 16 1024 1 0 0 0 0 0 0 traces/gcc_trace.txt > debug.op1
# diff -iw debug.op1 debug_runs/gcc.debug1.txt > diff.debug.op1
# echo "===================== gcc.debug1 ====================="
# cat diff.debug.op1
# rm -f debug.op1

# ./sim_cache 16 1024 2 0 0 8192 4 0 0 traces/gcc_trace.txt > debug.op2
# diff -iw debug.op2 debug_runs/gcc.debug2.txt > diff.debug.op2
# echo "===================== gcc.debug2 ====================="
# cat diff.debug.op2
# rm -f debug.op2

# ./sim_cache 16 1024 1 0 0 8192 4 0 0 traces/gcc_trace.txt > debug.op3
# diff -iw debug.op3 debug_runs/gcc.debug3.txt > diff.debug.op3
# echo "===================== gcc.debug3 ====================="
# cat diff.debug.op3
# rm -f debug.op3

rm -f debug.op4 debug.op5 debug.op6 debug.op7
rm -f diff.debug.op4 diff.debug.op5 diff.debug.op6 diff.debug.op7

./sim_cache 16 1024 1 1 4 8192 4 0 0 traces/gcc_trace.txt > debug.op4
diff -iw debug.op4 debug_runs/gcc.debug4.txt > diff.debug.op4
echo "===================== gcc.debug4 ====================="
cat diff.debug.op4
rm -f debug.op4

./sim_cache 16 1024 1 3 4 8192 4 0 0 traces/gcc_trace.txt > debug.op5
diff -iw debug.op5 debug_runs/gcc.debug5.txt > diff.debug.op5
echo "===================== gcc.debug5 ====================="
cat diff.debug.op5
rm -f debug.op5

./sim_cache 16 1024 1 2 4 8192 4 4 4 traces/gcc_trace.txt > debug.op6
diff -iw debug.op6 debug_runs/gcc.debug6.txt > diff.debug.op6
echo "===================== gcc.debug6 ====================="
cat diff.debug.op6
rm -f debug.op6

./sim_cache 16 1024 1 0 0 8192 4 3 8 traces/gcc_trace.txt > debug.op7
diff -iw debug.op7 debug_runs/gcc.debug7.txt > diff.debug.op7
echo "===================== gcc.debug7 ====================="
cat diff.debug.op7
rm -f debug.op7

