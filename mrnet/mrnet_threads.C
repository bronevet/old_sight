/****************************************************************************
* Copyright � Udayanga Wickramasinghe - Indiana University                 *
*                                                                          *
****************************************************************************/

#include "mrnet_iterator.h"
#include "sight_structure.h"
using namespace sight;
using namespace sight::structure;

#include "sight_merge.h"
using namespace sight::merge;
using namespace mrnstreaming;

#include "mrnet_merge.C"
using namespace mrnmergeing;
using namespace MRN;

void MRNetThread::init(std::set<Rank> ranks) {
    if (!initialized) {
        //initialize threads for producer and consumer
        // producer - callback thread (main)
        // consumer - merger thread
        // init conditions,mutexes
        //there are condition variable for each leaf rank, there are condition vars for each
        //create iterators
        //for each rank create iterator and relevant signal handlers
#ifdef DEBUG_ON
        fprintf(stdout, "[MRNetThread Initialization started.. PID : %d ]\n", getpid());
#endif
        //input/out sync handling
        inQueueMutex = new atomic_mutex_t;
        *inQueueMutex = ATOMIC_SYNC_MUTEX_INITIALIZER;

        int i = 0;

        for (std::set<Rank>::iterator ranks_it = ranks.begin(); ranks_it != ranks.end(); ranks_it++, i++) {
            Rank r = *ranks_it;
            std::vector<DataPckt> *inputQueue = new std::vector<DataPckt>;
            atomic_cond_t *signal = new atomic_cond_t;
            *signal = ATOMIC_SYNC_COND_INITIALIZER ;

            inputSignals[r] = signal;
            bufferData[r] = inputQueue;
            //each iterator owns a input queue
            //MRNetParser *it = new MRNetParser(*bufferData[r], inputSignals[r], inQueueMutex, synchronizer);
            baseStructureParser<FILE> *it = new MRNetParser(*bufferData[r], inputSignals[r], inQueueMutex, synchronizer);
            iterators.push_back(it);
        }

        //create a Merger and merge()
        //todo init merge obj properly


        //create therads
        thread1 = new pthread_t;
        pthread_create(thread1, NULL, &MRNetThread::consumerFuncHelper, this);

        //exit thread now
        initialized = true;
#ifdef DEBUG_ON
        fprintf(stdout, "[MRNetThread Initialization completed!!.. PID : %d rank ]\n", getpid());
#endif
    }
}

void *MRNetThread::consumerFuncHelper(void *arg) {
    MRNetThread *thisPtr = (MRNetThread *) arg;
    thisPtr->consumerFunc();
}

//void MRNetThread::consumerFunc() {
//#ifdef DEBUG_ON
//    fprintf(stdout, "[MRNetThread Merge thread Started.. PID : %d number of iterators : %d ]\n", getpid(), iterators.size());
//#endif
//    //do merging in this seperate thread
//    const char* outDir = "test_out/tmp";
//    mergeType mt = str2MergeType(string("zipper"));
//
//
//#ifdef VERBOSE
//  SightInit(argc, argv, "hier_merge", txt()<<outDir<<".hier_merge");
//  dbg << "#fileParserRefs="<<fileParsers.size()<<endl;
//  #else
//    SightInit_LowLevel();
//#endif
//
//    // Set the working directory in the dbgStreamMerger class (it is a static variable). This must be done before an instance of this class is created
//    // since the first and only instance of this class will read this working directory and write all output there.
//    dbgStreamMerger::workDir = outDir;
//
//    vector<pair<properties::tagType, const properties*> > emptyNextTag;
//
//    // Initialize the streamRecords for the incoming and outgoing stream. The variant
//    // ID starts as 0 as will grow deeper as we discover unmergeable differences
//    // between or within the input streams.
//    std::map<std::string, streamRecord*> outStreamRecords = MergeHandlerInstantiator::GetAllMergeStreamRecords(0);
//    dbgStreamStreamRecord::enterBlock(outStreamRecords);
//
//    std::vector<std::map<std::string, streamRecord*> > inStreamRecords;
//    for(int i=0; i<iterators.size(); i++) {
//        inStreamRecords.push_back(MergeHandlerInstantiator::GetAllMergeStreamRecords(i));
//    }
//    dbgStreamStreamRecord::enterBlock(inStreamRecords);
//
//    // Records that we're ready to read another tag from each parser
//    vector<bool> readyForTagFromAll(iterators.size(), true);
//    // Records that each parser is still active
//    vector<bool> allParsersActive(iterators.size(), true);
//
//    // Maps the next observed tag name/type to the input streams on which tags that match this signature were read
//    map<tagGroup2, StreamTags > tag2stream;
//
//    // Records the number of parsers on which the last read tag was text. We alternate between reading text
//    // and reading tags and if text is read from some but not all parsers, the contributions from the other
//    // parsers are considered to be the empty string.
//    int numTextTags = 0;
//
//#ifdef VERBOSE
//  graph g;
//  anchor outgoingA;
//#endif
//    merge(iterators, emptyNextTag, outStreamRecords, inStreamRecords,
//            readyForTagFromAll, allParsersActive,
//            tag2stream, numTextTags,
//            0, NULL, mt, strm, net, strm_id, tag_id,
//#ifdef VERBOSE
//        g, anchor::noAnchor, outgoingA,
//#endif
//            "");
//
//    // Close all the parsers and their files
////    for(vector<MRNetParser*>::iterator p=iterators.begin(); p!=iterators.end(); p++)
////        delete *p;
//#ifdef DEBUG_ON
//    fprintf(stdout, "[MRNetThread Merge thread END !!.. PID : %d number of iterators ]\n", getpid());
//#endif
//
//}

void MRNetThread::consumerFunc() {
#ifdef DEBUG_ON
    fprintf(stdout, "[MRNetThread Merge thread Started.. PID : %d number of iterators : %d ]\n", getpid(), iterators.size());
#endif
    //do merging in this seperate thread
    const char *outDir = "test_out/tmp";
    SightInit_LowLevel();
    // Set the working directory in the dbgStreamMerger class (it is a static variable). This must be done before an instance of this class is created
    // since the first and only instance of this class will read this working directory and write all output there.
    dbgStreamMerger::workDir = string(outDir);

#ifdef VERBOSE
    graph g;
    anchor outgoingA;
#endif

    MRNetMergeState state(iterators, strm, net, strm_id, tag_id
#ifdef VERBOSE
            , g, anchor::noAnchor, outgoingA
#endif
    );
    state.merge();

    //cleanup structures
    destroy();
}

void MRNetThread::destroy() {
//    delete thread1;
    delete inQueueMutex;
    delete synchronizer;
    int i = 0;
    for(std::vector<sight::baseStructureParser<FILE>*>::iterator it = iterators.begin(); it < iterators.end(); it++){
        sight::baseStructureParser<FILE>* parser = *it;
        delete parser ;
        delete inputSignals[i];
        delete bufferData[i];
        i++;
    }

    //todo remove all heap buffers and signals ,etc safely
}


