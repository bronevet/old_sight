make
NPROCS=8
rm -rf dbg*
export SIGHT_FILE_OUT=1
mpirun -np $NPROCS ex

rm -f distances.txt
for ((VAR1=1; VAR1<$NPROCS; VAR1++))
do
for ((VAR2=0; VAR2<$VAR1; VAR2++))
do

rm -rf dbgOut

echo $VAR1 $VAR2 
#>> distances.txt
eval VAR3=`find dbg.rank_$VAR1/ -type f -name 'structure' -print0 | du -cb --files0-from=- | grep "total" | awk '{print $1}'`
eval VAR4=`find dbg.rank_$VAR2/ -type f -name 'structure' -print0 | du -cb --files0-from=- | grep "total" | awk '{print $1}'`

#find dbgOut/ -type f -name 'structure' -print0 | du -cb --files0-from=- | grep "total" | awk '{print $1}'

export SIGHT_FILE_OUT=1
/g/g92/polyzou1/pro_sight/sight/hier_merge dbgOut common dbg.rank_$VAR1/structure dbg.rank_$VAR2/structure

eval VAR=`find dbgOut/ -type f -name 'structure' -print0 | du -cb --files0-from=- | grep "total" | awk '{print $1}'`

(( $VAR3 <= $VAR4 )) &&  eval var=$VAR4 || eval var=$VAR3
echo "$VAR $var" | awk '{printf "%.4f \n", $1/($2-12000)}' >> distances.txt
 
#echo $var >> distances.txt

done
done

#/g/g92/polyzou1/pro_sight/sight/slayout dbgOut/structure;