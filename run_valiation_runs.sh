#!/bin/sh

make clean ; make 

rm -f validation.op0 validation.op1 validation.op2 validation.op3
rm -f diff.validation.op0 diff.validation.op1 diff.validation.op2 diff.validation.op3

./sim_cache 16 1024 2 0 0 0 0 0 0 traces/gcc_trace.txt > validation.op0
diff -iw validation.op0 validation_runs/gcc.output0.txt > diff.validation.op0
echo "===================== gcc.validation0 ====================="
cat diff.validation.op0
rm -f validation.op0

./sim_cache 16 1024 1 0 0 0 0 0 0 traces/gcc_trace.txt > validation.op1
diff -iw validation.op1 validation_runs/gcc.output1.txt > diff.validation.op1
echo "===================== gcc.validation1 ====================="
cat diff.validation.op1
rm -f validation.op1

./sim_cache 16 1024 2 0 0 8192 4 0 0 traces/gcc_trace.txt > validation.op2
diff -iw validation.op2 validation_runs/gcc.output2.txt > diff.validation.op2
echo "===================== gcc.validation2 ====================="
cat diff.validation.op2
rm -f validation.op2

./sim_cache 16 1024 1 0 0 8192 4 0 0 traces/gcc_trace.txt > validation.op3
diff -iw validation.op3 validation_runs/gcc.output3.txt > diff.validation.op3
echo "===================== gcc.validation3 ====================="
cat diff.validation.op3
rm -f validation.op3

rm -f validation.op4 validation.op5 validation.op6 validation.op7
rm -f diff.validation.op4 diff.validation.op5 diff.validation.op6 diff.validation.op7

./sim_cache 16 1024 1 1 4 8192 4 0 0 traces/gcc_trace.txt > validation.op4
diff -iw validation.op4 validation_runs/gcc.output4.txt > diff.validation.op4
echo "===================== gcc.validation4 ====================="
cat diff.validation.op4
rm -f validation.op4

./sim_cache 16 1024 1 3 4 8192 4 0 0 traces/gcc_trace.txt > validation.op5
diff -iw validation.op5 validation_runs/gcc.output5.txt > diff.validation.op5
echo "===================== gcc.validation5 ====================="
cat diff.validation.op5
rm -f validation.op5

./sim_cache 16 1024 1 2 4 8192 4 4 4 traces/gcc_trace.txt > validation.op6
diff -iw validation.op6 validation_runs/gcc.output6.txt > diff.validation.op6
echo "===================== gcc.validation6 ====================="
cat diff.validation.op6
rm -f validation.op6

./sim_cache 16 1024 1 0 0 8192 4 3 8 traces/gcc_trace.txt > validation.op7
diff -iw validation.op7 validation_runs/gcc.output7.txt > diff.validation.op7
echo "===================== gcc.validation7 ====================="
cat diff.validation.op7
rm -f validation.op7

