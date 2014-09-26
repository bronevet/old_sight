#make
#export SIGHT_FILE_OUT=1
#mpirun -np 4 ex
#export SIGHT_FILE_OUT=1
#/g/g92/polyzou1/pro_sight/sight/hier_merge dbgOut zipper dbg.rank_*/structure

#/g/g92/polyzou1/pro_sight/sight/slayout dbgOut/structure;

make
export SIGHT_FILE_OUT=1
mpirun -np 4 ex
export SIGHT_FILE_OUT=1
#/g/g92/polyzou1/pro_sight/sight/hier_merge dbgOut1 common dbg.rank_1/structure dbg.rank_2/structure dbg.rank_3/structure
#/g/g92/polyzou1/pro_sight/sight/slayout dbgOut1/structure;

export SIGHT_MERGE_SEPARATE=1
/g/g92/polyzou1/pro_sight/sight/hier_merge dbgOut zipper dbg.rank_*/structure
#														 dbgOut1/structure dbg.rank_0/structure

/g/g92/polyzou1/pro_sight/sight/slayout dbgOut/structure;

#copy files out
#rm -r ../htmlfiles/*
#cp dbgOut/index.html ../htmlfiles -r
#cp dbgOut/html ../htmlfiles -r
#tar -cvf ../file.tar ../htmlfiles/

#copy files out
rm -rf ../htmlfiles/example6
mkdir ../htmlfiles/example6
cp dbgOut/index.html ../htmlfiles/example6/   
cp -r dbgOut/html ../htmlfiles/example6/