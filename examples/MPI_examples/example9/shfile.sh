make
export SIGHT_FILE_OUT=1
mpirun -np 4 ex
export SIGHT_FILE_OUT=1
/g/g92/polyzou1/pro_sight/sight/hier_merge dbgOut zipper dbg.rank_*/structure

/g/g92/polyzou1/pro_sight/sight/slayout dbgOut/structure;
