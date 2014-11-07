#make
#export SIGHT_FILE_OUT=1
#mpirun -np 4 ex
#export SIGHT_FILE_OUT=1
#/g/g92/polyzou1/pro_sight/sight/hier_merge dbg.Out1 common dbg.rank_0/structure dbg.rank_1/structure 
#/g/g92/polyzou1/pro_sight/sight/hier_merge dbg.Out2 common dbg.rank_2/structure dbg.rank_3/structure 

#dbg.rank_3/structure
#/g/g92/polyzou1/pro_sight/sight/slayout dbgOut1/structure;

#export SIGHT_MERGE_SEPARATE=1
#/g/g92/polyzou1/pro_sight/sight/hier_merge dbgOut zipper dbg.Out*/structure

#dbgOut1/structure dbg.rank_0/structure

make
export SIGHT_FILE_OUT=1
mpirun -np 4 ex
export SIGHT_FILE_OUT=1
export SIGHT_MERGE_SEPARATE=1
/g/g92/polyzou1/pro_sight/sight/hier_merge dbgOut zipper dbg.rank_*/structure

/g/g92/polyzou1/pro_sight/sight/slayout dbgOut/structure;

#copy files out
rm -rf ../htmlfiles/example11
mkdir ../htmlfiles/example11
cp dbgOut/index.html ../htmlfiles/example11/   
cp -r dbgOut/html ../htmlfiles/example11/