/****************************************************************************
* Copyright � Udayanga Wickramasinghe - Indiana University                 *
*                                                                          *
****************************************************************************/

#include "mrnet_iterator.h"
#include "mrnet_integration.h"

using namespace mrnstreaming;
using namespace MRN;

/**
* produces incoming raw filter data to the input queue as a character stream
*/
void MRNetProducer::loadBuffer(std::vector< PacketPtr > &packets_in, const TopologyLocalInfo &info) {
#ifdef DEBUG_ON
    printf("[LOAD METHOD of Input Producer...pid : %d size : %d ] \n", getpid(), packets_in.size());
#endif
    unsigned length;
    for (unsigned int i = 0; i < packets_in.size(); i++) {
        PacketPtr curr_packet = packets_in[i];
        Rank cur_inlet_rank = curr_packet->get_InletNodeRank();
        if (cur_inlet_rank == -1) {
            be_node = true;
        }
        //handle special case - BE sync and case where node is down
        if (cur_inlet_rank != UNKNOWN_NODE && packets_in.size() != 1) {
            if (net->node_Failed(cur_inlet_rank)) {
#ifdef DEBUG_ON
                printf("[NODE FAILED ] \n");
#endif
                // drop packets from failed node
                continue;
            }
        }
        char* val;
        curr_packet->unpack("%ac", &val, &length);

        //Start locking the queue
        synchronizer->set_mutex_lock(inQueueMutex);
        std::map<Rank, std::vector<DataPckt> *>::iterator it = this->bufferData.find(cur_inlet_rank);

        //if no packets already present in vector array then create new one
        if (it == this->bufferData.end()) {
            this->bufferData[cur_inlet_rank] = new std::vector<DataPckt>;
        }

        int tag_id = curr_packet->get_Tag();
        bool is_final = false ;
        if(tag_id == PROT_END_PHASE){
            is_final = true;
        }
        DataPckt* pkt = new DataPckt(val, length, is_final);
//        pkt->printData();
//        sleep(3);
        //insert unpacked values to relevant buffer indexed by rank
        this->bufferData[cur_inlet_rank]->push_back(*pkt);

        //signal the relevant input consumer  - should be done within lock
        std::map<Rank, atomic_cond_t *>::iterator q = this->inQueueSignals.find(cur_inlet_rank);
        atomic_cond_t *cond = q->second;
        //todo check if handled properly
        requestsByRank[cur_inlet_rank] += 1;
        synchronizer->set_cond_signal(cond);
        //unlock the input queue
        synchronizer->set_mutex_unlock(inQueueMutex);

    }
#ifdef DEBUG_ON
    printf("[LOAD METHOD Done !!...pid : %d  ] \n", getpid());
#endif
}

