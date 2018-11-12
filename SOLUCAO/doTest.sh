#!/bin/bash
threads=$1
txt_path=$2

echo "#threads,exec_time,speedup" >> $txt_path.speedups.csv

./CircuitRouter-SeqSolver/CircuitRouter-SeqSolver $txt_path
t_sequencial="$(grep "Elapsed time" $txt_path.res | cut -c "19-26")"
echo 1S,$t_sequencial,1 >> $txt_path.speedups.csv

counter=1
until [ $counter = $((threads+1)) ]
do
  ./CircuitRouter-ParSolver/CircuitRouter-ParSolver -t $counter $txt_path
  t_paralela="$(grep "Elapsed time" $txt_path.res | cut -c "19-26")"
  speedup=$(echo "scale = 6; $t_sequencial / $t_paralela" | bc)
  echo $counter,$t_paralela,$speedup >> $txt_path.speedups.csv
  counter=$((counter+1))
done
