make
export SIGHT_FILE_OUT=1
mpirun -np 8 ex
export SIGHT_FILE_OUT=1
export SIGHT_MERGE_SEPARATE=1
/g/g92/polyzou1/pro_sight/sight/hier_merge dbgOut zipper dbg.rank_*/structure

/g/g92/polyzou1/pro_sight/sight/slayout dbgOut/structure;

#copy files out
#rm -rf ../htmlfiles/example21
#mkdir ../htmlfiles/example21
#cp dbgOut/index.html ../htmlfiles/example21/   
#cp -r dbgOut/html ../htmlfiles/example21/

#find dbgOut/ -type f -name 'structure' -print0 | du -cb --files0-from=- | grep "total" | awk '{print $1}'