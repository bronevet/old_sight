/****************************************************************************
* Copyright � Udayanga Wickramasinghe - Indiana University                 *
*                                                                          *
****************************************************************************/

#if !defined(mrnet_iterator_h )
#define mrnet_iterator_h 1

#include "mrnet/Types.h"
#include <unistd.h>
#include <mrnet/Stream.h>
#include <pthread.h>
#include "AtomicSyncPrimitives.h"
#include "../process.h"
#include "mrnet_integration.h"

using namespace MRN;
using namespace std;
using namespace atomiccontrols;
using namespace sight;

namespace mrnstreaming {
    /**
    *   Functionality encapsulated for the producer queue
    */
    class MRNetProducer{
        //stores input data
        std::map<Rank, std::vector<DataPckt>*> bufferData;
        std::map<Rank, int> requestsByRank;
        AtomicSync* synchronizer ;

        //sync primitives
        std::map<Rank, atomic_cond_t*> inQueueSignals;
        atomic_mutex_t* inQueueMutex;

        //MRnet specific params
        Stream* stream;
        Network *net;
        std::set<Rank> peers;
        int wave ;

    public:
        MRNetProducer(std::map<Rank, std::vector<DataPckt>*> bufferData , atomic_mutex_t* inQueueMutex, std::map<Rank, atomic_cond_t*> inQueueSignals,
                std::set<Rank> peers, Stream* stream, Network *net, AtomicSync* s){
            this->bufferData = bufferData;
            this->inQueueMutex = inQueueMutex;
            this->inQueueSignals = inQueueSignals;
            this->stream = stream;
            this->net = net;
            this->peers = peers ;
            this->synchronizer = s ;
//            no_of_ranks = peers.size();
            for(std::set<Rank>::iterator it = peers.begin() ; it != peers.end() ; it++){
                requestsByRank[(*it)] = 0 ;
            }
            wave = 0 ;
        }

        /**
        * produces incoming raw filter data to the input queue as a character stream
        */
        void loadBuffer(std::vector< PacketPtr > &packets_in, const TopologyLocalInfo& info);
        bool be_node;
    }   ;

    /**
    *   Functionality encapsulated for the consumer of streams
    */
    class MRNetThread {
    private:
        //stores input data
        std::map<Rank, std::vector<DataPckt>*> bufferData;
        pthread_t* thread1 ;
        /**
        * We can have number of base structure parsers for parsing the input from different filter streams
        * When MRNet start sending filterred data each stream is directed to an incoming data queue
        * An MRnet iterator is bound to each of the queue which will return sight information (merged
        * or non merged) to the mrnet merging layer
        */
        std::vector<sight::baseStructureParser<FILE>*> iterators;

        //sync primitives
        std::map<Rank, atomic_cond_t*> inputSignals ;
        AtomicSync* synchronizer ;
        atomic_mutex_t* inQueueMutex;

        bool initialized;

        //MRNEt specific objects
        Stream *strm;
        int strm_id;
        int tag_id;
        Network *net;

    public:
        MRNetThread(AtomicSync* s, Stream* strm, int stream_id, int tag_id, Network* net):initialized(false){
            this->synchronizer = s ;
            this->strm = strm;
            this->strm_id = stream_id;
            this->tag_id = tag_id ;
            this->net = net;
        }

        /**
        * initialize structures for parsing and setup consumer thread
        */
        void init(std::set<Rank> ranks);

        /*
         * clean up used up buffer at exit
          * */
        void destroy();

        /**
        * actual consumer functionality is defined here
        */
        void consumerFunc();

        /**
        * Helper function to avoid a issue with pthread functionality - 'this' is not visible in pthread_create()
        * use a static function which call the actual member function
        */
        static void* consumerFuncHelper(void *arg);

        std::map<Rank, std::vector<DataPckt>*> getInputBuffer(){
            return bufferData;
        };

        std::map<Rank, atomic_cond_t*> getPerRankSignals(){
            return inputSignals;
        };

        atomic_mutex_t* getInputMutex(){
            return inQueueMutex;
        }

    } ;

}
#endif /* integer_addition_h */
