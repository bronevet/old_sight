/****************************************************************************
* Copyright � Udayanga Wickramasinghe - Indiana University                 *
*                                                                          *
****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <list>
#include <iostream>
#include <string>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "../utils.h"
#include "../process.h"
//#include "process.C"
#include "../sight_structure.h"
#include "../sight_merge.h"
using namespace std;
using namespace sight;
using namespace sight::structure;
using namespace sight::merge;

namespace mrnmergeing{

    /**
    * Declaration of MRNet based Merge State class
    * main role of this class is to store state required to merge MRNet based stream handling.
    * many of the functionality is inherited from MergeState which will do the actual merging, however
    * at the point of generating the output , the members/functions of MRNetMergeState would override
    * the defualt behaviour
    */
    class MRNetMergeState : public MergeState{
    private:
        MRN::Stream *strm;
        int strm_id;
        int tag_id;
        MRN::Network *net;

    public:
        MRNetMergeState(const vector<baseStructureParser<FILE>*>& parsers, MRN::Stream* strm, MRN::Network* net, int strm_id, int tag_id
#ifdef VERBOSE
                , graph& g, anchor incomingA, anchor outgoingA
#endif
           ):MergeState( parsers
#ifdef VERBOSE
                , g, incomingA, outgoingA
#endif
        ){
            this->strm = strm ;
            this->net = net ;
            this->strm_id = strm_id;
            this->tag_id = tag_id;
            //special setting enable interleaved Merge only
            tagGroup::useInterleavedMergeOnly = true ;
        }

        /**
        * create dbgStream specific for MRNet transfer
        */
        structure::dbgStream* createStream(properties* props, bool storeProps){
            properties::iterator sightIt = props->find("sight");
            assert(!sightIt.isEnd());

            // Create the directory structure for the structural information
            // Main output directory
            createDir(properties::get(sightIt, "workDir"), "");

            // Directory where client-generated images will go
            string imgDir = createDir(properties::get(sightIt, "workDir"), "html/dbg_imgs");

            // Directory that widgets can use as temporary scratch space
            string tmpDir = createDir(properties::get(sightIt, "workDir"), "html/tmp");

            //use a MRNet specific stream buffer
            mrnBuf* mrnBuffer = new mrnBuf(strm, net, strm_id, tag_id);
//  return new MRNetostream(strm, net, strm_id, tag_id, storeProps? props: NULL, properties::get(sightIt, "title"), properties::get(sightIt, "workDir"), imgDir, tmpDir);
            return new MRNetostream(mrnBuffer, storeProps? props: NULL, properties::get(sightIt, "title"), properties::get(sightIt, "workDir"), imgDir, tmpDir);
        }


    }  ;


}
