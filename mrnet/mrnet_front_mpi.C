/****************************************************************************
* Copyright � Udayanga Wickramasinghe - Indiana University                 *
*                                                                          *
****************************************************************************/
#include "mrnet_front.h"
#include "mpi.h"

bool saw_failure = false;

int num_attach_callbacks = 0;
int num_detach_callbacks = 0;

atomic_mutex_t cb_lock = ATOMIC_SYNC_MUTEX_INITIALIZER;
AtomicSync locker ;

void BE_Add_Callback( Event* evt, void* evt_data )
{
    if( (evt->get_Class() == Event::TOPOLOGY_EVENT) &&
            (evt->get_Type() == TopologyEvent::TOPOL_ADD_BE) ) {
        locker.set_mutex_lock(&cb_lock);
        num_attach_callbacks++;
        locker.set_mutex_unlock(&cb_lock);

        TopologyEvent::TopolEventData* ted = (TopologyEvent::TopolEventData*) evt_data;
        delete ted;
    }
}
void BE_Remove_Callback( Event* evt, void* evt_data )
{
    if( (evt->get_Class() == Event::TOPOLOGY_EVENT) &&
            (evt->get_Type() == TopologyEvent::TOPOL_REMOVE_NODE) ) {
        locker.set_mutex_lock(&cb_lock);
        num_detach_callbacks++;
        locker.set_mutex_unlock(&cb_lock);

        TopologyEvent::TopolEventData* ted = (TopologyEvent::TopolEventData*) evt_data;
        delete ted;
    }
}

string createDir(string workDir, string dirName) {
    ostringstream fullDirName; fullDirName<<workDir<<"/"<<dirName;
    ostringstream cmd; cmd << "mkdir -p "<<fullDirName.str();
    int ret = system(cmd.str().c_str());
    if(ret == -1) { cout << "ERROR creating directory \""<<workDir<<"/"<<dirName<<"\"!"; exit(-1); }
    return fullDirName.str();
}

void printLeafInfo(std::vector< NetworkTopology::Node * >& leaves, int num_be, int comm_nodes , char* conn_info){
    unsigned num_leaves = unsigned(leaves.size());
    unsigned be_per_leaf = num_be / num_leaves;
    unsigned curr_leaf = 0;
    FILE *connFile = NULL ;
    if (conn_info != NULL) {
        connFile = fopen(conn_info, "w");
    }
    for (unsigned i = 0; (i < num_be) && (curr_leaf < num_leaves); i++) {
        if (i && (i % be_per_leaf == 0)) {
            // select next parent
            curr_leaf++;
            if (curr_leaf == num_leaves) {
                // except when there is no "next"
                curr_leaf--;
            }
        }
        fprintf(stdout, "BE %d will connect to %s:%d:%d\n",
                i,
                leaves[curr_leaf]->get_HostName().c_str(),
                leaves[curr_leaf]->get_Port(),
                leaves[curr_leaf]->get_Rank());
        if (conn_info != NULL){
            fprintf(connFile, "%s \t %d \t %d\n",
                leaves[curr_leaf]->get_HostName().c_str(),
                leaves[curr_leaf]->get_Port(),
                leaves[curr_leaf]->get_Rank());
        }

        char port_str[15], rank_str[15];
        snprintf(port_str, sizeof(port_str), "%d",leaves[curr_leaf]->get_Port());
        snprintf(rank_str, sizeof(rank_str), "%d",leaves[curr_leaf]->get_Rank());
        //send parent connection info to BE app [i]
        int target_be_rank = comm_nodes + i + 1;
        MPI_Send(leaves[curr_leaf]->get_HostName().c_str(), 200, MPI_CHAR, target_be_rank, 1001, MPI_COMM_WORLD);
        MPI_Send(port_str, 200, MPI_CHAR, target_be_rank, 1002, MPI_COMM_WORLD);
        MPI_Send(rank_str, 200, MPI_CHAR, target_be_rank, 1003, MPI_COMM_WORLD);

    }

    if (conn_info != NULL) {
        fflush(connFile);
        fclose(connFile);
    }
}

int startup(char* topology_file, char* so_file, char*  structureFile_path, int num_commnodes, int num_backends)
{
    int nets = 1;
    int send_val=32;
    int recv_val;
    int tag, retval;
    PacketPtr p;
    //open output file
    FILE* structureFile = fopen (structureFile_path,"w");

    int n = 0;
    while( n++ < nets ) {

        saw_failure = false;

        if( nets > 1 )
            fprintf(stdout, "\n\n---------- Network Instance %d ----------\n\n", n);

#ifdef DEBUG_ON
        fprintf(stdout, "PID: %d top : %s , BE : %s dummy : %s num BEs : %d \n", getpid(),
                topology_file, "no initialized BE", dummy_argv, num_backends);
#endif
        // If backend_exe (2nd arg) and backend_args (3rd arg) are both NULL,
        // then all nodes specified in the topology are internal tree nodes.
        Network * net = Network::CreateNetworkFE( topology_file, NULL, NULL );
        if( net->has_Error() ) {
            net->perror("Network creation failed!!!");
            exit(-1);
        }

        if( ! net->set_FailureRecovery(false) ) {
            fprintf( stdout, "Failed to disable failure recovery\n" );
            delete net;
            return -1;
        }

        bool cbrett = net->register_EventCallback( Event::TOPOLOGY_EVENT,
                TopologyEvent::TOPOL_ADD_BE,
                BE_Add_Callback, NULL );
        if(cbrett == false) {
            fprintf( stdout, "Failed to register callback for back-end add topology event\n");
            delete net;
            return -1;
        }
        cbrett = net->register_EventCallback( Event::TOPOLOGY_EVENT,
                TopologyEvent::TOPOL_REMOVE_NODE,
                BE_Remove_Callback, NULL );
        if(cbrett == false) {
            fprintf( stdout, "Failed to register callback for back-end remove topology event\n");
            delete net;
            return -1;
        }

        // Query net for topology object
        NetworkTopology * topology = net->get_NetworkTopology();
        std::vector< NetworkTopology::Node * > internal_leaves;
        topology->get_Leaves(internal_leaves);
        topology->print(stdout);

        //print leaf info
        printLeafInfo(internal_leaves, num_backends, num_commnodes, NULL);

        // Wait for backends to attach
        unsigned int waitfor_count = num_backends;
        fprintf( stdout, "FE PID : %d Please start backends now.\n\nWaiting for %u backends to connect\n",
                getpid(), waitfor_count );
        fflush(stdout);
        unsigned curr_count = 0;
        do {
            sleep(1);
            locker.set_mutex_lock(&cb_lock);
            curr_count = num_attach_callbacks;
            locker.set_mutex_unlock(&cb_lock);
            fprintf( stdout, " %d backends have been attached!\n", curr_count);
        } while( curr_count != waitfor_count );
#ifdef DEBUG_ON
        fprintf( stdout, "All %u backends have attached!\n", waitfor_count);
#endif
//        Make sure path to "so_file" is in LD_LIBRARY_PATH
        // Load the filter handler for MRNet by the name of "SightStreamAggregator"
        int filter_id = net->load_FilterFunc( so_file, "SightStreamAggregator" );
        if( filter_id == -1 ){
            fprintf( stderr, "Network::load_FilterFunc() failure\n" );
            delete net;
            return -1;
        }

        // A Broadcast communicator contains all the back-ends
        Communicator * comm_BC = net->get_BroadcastCommunicator( );

        // Create a stream that will use the "SightStreamAggregator"filter for aggregation
        //Also disable default synchronization filter -> SFILTER_DONTWAIT
        Stream * add_stream = net->new_Stream( comm_BC,
                                               filter_id,
//                                               TFILTER_SUM,
//                                               SFILTER_WAITFORALL );
                SFILTER_DONTWAIT );

        int num_backends2 = int(comm_BC->get_EndPoints().size());

        // Broadcast a control message to back-ends to send us "num_iters"
        // waves of integers
        tag = PROT_CONCAT;
        //total number of waves are calculated using --> total number of integers we like to send / number of intergers per wave
#ifdef DEBUG_ON
        fprintf( stdout, "preparing to send INIT tags.. num_be : %d\n", num_backends2);
#endif
        unsigned int num_iters= TOTAL_STREAM_SIZE / TOTAL_PACKET_SIZE;

        if( add_stream->send( tag, "%d %d", send_val, num_iters ) == -1 ){
            fprintf( stderr, "stream::send() failure\n" );
            return -1;
        }
        if( add_stream->flush( ) == -1 ){
            fprintf( stderr, "stream::flush() failure\n" );
            return -1;
        }
#ifdef DEBUG_ON
        fprintf( stdout, "INIT tag flush() done... \n");
#endif

/*
* Main  loop where merged output is recieved from child nodes
* */
        char* recv_Ar;
        unsigned length;
        int total_len = 0 ;
        // We expect "num_iters" aggregated responses from all back-ends
        while(true){

            retval = add_stream->recv(&tag, p);
#ifdef DEBUG_ON
            fprintf(stdout, "\n[FE: STREM->recv done ; retval : %d] \n", retval);
#endif
            if(tag == PROT_END_PHASE){
                //print any stream coming with protocol end phase
#ifdef DEBUG_ON
                fprintf(stdout, "FE: Iteration PROTOCOL END SUCCESS %d: \n", 0);
#endif
                if( p->unpack( "%ac", &recv_Ar, &length ) == -1 ){
                    fprintf( stderr, "PROTOCOL END stream::unpack() failure\n" );
                    return -1;
                }
                total_len+= length;
                for(int j = 0 ; j < length ; j++){
                    fprintf(structureFile, "%c", recv_Ar[j]);
                }
                fprintf(stdout, "\n[FE: PROTOCOL SUCCESS: Output stored: bytes written => most recent  : [%d] total : [%d] ] \n",
                        length, total_len);
                fflush(structureFile);
                break;
            }
            if( retval == 0 ) {
                //shouldn't be 0, either error or block for data, unless a failure occured
                fprintf( stderr, "stream::recv() returned zero\n" );
                if( saw_failure ) break;
                return -1;
            }
            if( retval == -1 ) {
                //recv error
                fprintf( stderr, "stream::recv() unexpected failure\n" );
                if( saw_failure ) break;
                return -1;
            }

            if( p->unpack( "%ac", &recv_Ar, &length ) == -1 ){
                fprintf( stderr, "stream::unpack() failure\n" );
                return -1;
            }

            total_len += length;
            for(int j = 0 ; j < length ; j++){
                fprintf(structureFile, "%c", recv_Ar[j]);
            }
//            fprintf(stdout, "\n[FE: Display values done!] \n");

        }

        if( saw_failure ) {
            fprintf( stderr, "FE: a network process has failed, killing network\n" );
            fflush(structureFile);
            fclose(structureFile);
            delete net;
        }
        else {
            delete add_stream;
            fflush(structureFile);
            fclose(structureFile);
            // Tell back-ends to exit
            Stream * ctl_stream = net->new_Stream( comm_BC, TFILTER_MAX,
                    SFILTER_WAITFORALL );
            if(ctl_stream->send(PROT_EXIT, "") == -1){
                fprintf( stderr, "stream::send(exit) failure\n" );
                return -1;
            }
            if(ctl_stream->flush() == -1){
                fprintf( stderr, "stream::flush() failure\n" );
                return -1;
            }
            retval = ctl_stream->recv(&tag, p);
            if( retval == -1){
                //recv error
                fprintf( stderr, "stream::recv() failure\n" );
                return -1;
            }
            delete ctl_stream;
            if( tag == PROT_EXIT ) {
                // The Network destructor will cause all internal and leaf tree nodes to exit
                delete net;
                break;
            }
        }
    }


    return 0;
}
