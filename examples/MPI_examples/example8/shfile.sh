make
export SIGHT_FILE_OUT=1
mpirun -np 4 ex
export SIGHT_FILE_OUT=1
export SIGHT_MERGE_SEPARATE=1
/g/g92/polyzou1/pro_sight/sight/hier_merge dbgOut zipper dbg.rank_*/structure


#/g/g92/polyzou1/pro_sight/sight/hier_merge dbgOut1 zipper dbg.rank_1/structure dbg.rank_2/structure 
#dbg.rank_3/structure
#/g/g92/polyzou1/pro_sight/sight/slayout dbgOut1/structure;

#export SIGHT_MERGE_SEPARATE=1
#/g/g92/polyzou1/pro_sight/sight/hier_merge dbgOut zipper dbg.rank_0/structure dbgOut1/structure dbg.rank_3/structure

/g/g92/polyzou1/pro_sight/sight/slayout dbgOut/structure;

#rm -r ../htmlfiles/*
#cp dbgOut/index.html ../htmlfiles -r
#cp dbgOut/html ../htmlfiles -r

#copy files out
rm -rf ../htmlfiles/example8
mkdir ../htmlfiles/example8
cp dbgOut/index.html ../htmlfiles/example8/   
cp -r dbgOut/html ../htmlfiles/example8/