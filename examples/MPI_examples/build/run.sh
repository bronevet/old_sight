export SIGHT_FILE_OUT=1
mpirun -np $2 ../bin/$1
export SIGHT_FILE_OUT=1
export SIGHT_MERGE_SEPARATE=1
mkdir -p ../output/$1
/g/g92/polyzou1/pro_sight/sight/hier_merge ../output/$1 zipper dbg.rank_*/structure

/g/g92/polyzou1/pro_sight/sight/slayout ../output/$1/structure;
#rm -rf dbg.rank_*