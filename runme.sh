#!/bin/sh

echo "============= Performing the Validation Runs ============="
./run_valiation_runs.sh
echo "=========================================================="

echo "=============== Performing the Debug Runs ================"
./run_debug_runs.sh
echo "=========================================================="

echo "============= Simulations to generate data for the Graphs ============"
make clean ; make

echo "======== Graph #1 ======== "
./graph1.sh
num=`cat points.1 | wc -l`
num=`expr $num - 5`
echo "Number of points in Graph1: $num"
echo "========================== "

echo "======== Graph #2 ======== "
./graph2.sh
num=`cat points.1 | wc -l`
num=`expr $num - 5`
echo "Number of points in Graph1: $num"
echo "========================== "

echo "======== Graph #3 ======== "
./graph3.sh
num=`cat points.3 | wc -l`
num=`expr $num - 5`
echo "Number of points in Graph3: $num"
echo "========================== "

echo "======== Graph #4 ======== "
./graph4.sh
num=`cat points.4 | wc -l`
num=`expr $num - 5`
echo "Number of points in Graph4: $num"
echo "========================== "

echo "======== Graph #5 ======== "
./graph5.sh
num=`cat points.5 | wc -l`
num=`expr $num - 7`
echo "Number of points in Graph5: $num"
echo "========================== "

echo "======== Graph #6 ======== "
./graph6.sh
num=`cat points.6 | wc -l`
num=`expr $num - 16`
echo "Number of points in Graph6: $num"
echo "========================== "

