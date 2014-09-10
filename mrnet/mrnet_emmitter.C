/****************************************************************************
* Copyright � Udayanga Wickramasinghe - Indiana University                 *
*                                                                          *
****************************************************************************/
#include "mrnet/MRNet.h"
#include "mrnet_integration.h"

using namespace MRN;
/**
* acts as the Stream emitter for MRNet backend
* if MRNET_MERGE_EXEC ENV var is defined  (at #sightInit()) then it will execute this process
*/
int main(int argc, char **argv)
{
    // Command line args are: parent_hostname, parent_port, parent_rank, my_hostname, my_rank
    // these are used to instantiate the backend network object

    if( argc != 6 ) {
        fprintf(stderr, "Incorrect usage, must pass parent/local info\n");
        return -1;
    }

    const char* parHostname = argv[argc-5];
    Port parPort = (Port)strtoul( argv[argc-4], NULL, 10 );
    const char* myHostname = argv[argc-2];
    Rank myRank = (Rank)strtoul( argv[argc-1], NULL, 10 );
    Rank parRank = (Rank)strtoul( argv[argc-3], NULL, 10 );

    fprintf( stdout, "BE pid : [%d] host:rank %s:%d connecting to parent host:port:rank %s:%d:%d\n",
            getpid(),myHostname, myRank, parHostname, parPort, parRank);

    Stream * stream = NULL;
    PacketPtr p;
    int rc, tag=0, num_iters=0;
    int send_val = 10 ;
    int recv_val = 0;
    int wave = 0;

    char* sendAr = (char*)malloc(TOTAL_PACKET_SIZE);

    Network * net = Network::CreateNetworkBE( argc, argv );

    do {
        //configure for piped input from emitter
        FILE* f = stdin;
        //wait until inidacated by FE
#ifdef DEBUG_ON
        printf("BE waiting to recv() pid: %d \n", getpid());
#endif
        rc = net->recv(&tag, p, &stream);
        if( rc == -1 ) {
            fprintf( stderr, "BE: Network::recv() failure in emmmitter recv_val : %d num_iters : %d\n", recv_val, num_iters );
            break;
        }
        else if( rc == 0 ) {
//            a stream was closed
            continue;
        }

         
        switch(tag) {

        case PROT_CONCAT:
            p->unpack( "%d %d", &recv_val, &num_iters );

            printf("Init BE pid : %d : recieved token :%d\n", getpid(),recv_val);
                // Send integer arrays as waves - simulate buffer waves
                for (int i = 0; i < num_iters; i++) {
                    while (!feof(f)) {
                        size_t bytes_read = populateBuffer(sendAr, f);
#ifdef DEBUG_ON
                        fprintf(stdout, "BE: Sending wave %d ..\n", wave++);
                        fprintf(stdout, "BE: Sending wave wait send Auc.. bytes read : %d \n", bytes_read);

                        for(int j = 0 ; j < bytes_read ; j++){
                            printf("%c",sendAr[j]);
                        }
                        printf("\n\n\n");
#endif
                        if (stream->send(tag, "%ac", sendAr, bytes_read) == -1) {
                            fprintf(stderr, "BE: stream::send(%%d) failure in PROT_CONCAT\n");
                            tag = PROT_EXIT;
                            break;
                        }
                        if (stream->flush() == -1) {
                            fprintf(stderr, "BE: stream::flush() failure in PROT_CONCAT\n");
                            break;
                        }
#ifdef DEBUG_ON
                        fprintf(stdout, "BE: Sending wave Done..... \n");
                        fflush(stdout);
#endif
                        sleep(2); // stagger sends
                    }

                    //End of stream go to exit stage
                    //todo handle this properly - introduce new state ? ie:- PROT_EOF_STREAM
                    char* dummy = (char*) malloc(1);
                    *dummy = ' ';
                    if (stream->send(PROT_END_PHASE, "%ac", dummy, 1) == -1) {
                            fprintf(stderr, "BE: stream::send(%%d) failure in PROT_CONCAT\n");
                            tag = PROT_EXIT;
                    }
                    break;
                }
                break;

        case PROT_EXIT:
            if( stream->send(tag, "%d", 0) == -1 ) {
                fprintf( stderr, "BE: stream::send(%%s) failure in PROT_EXIT\n" );
                break;
            }
            if( stream->flush( ) == -1 ) {
                fprintf( stderr, "BE: stream::flush() failure in PROT_EXIT\n" );
            }
            fprintf(stdout, "BE: pid : %d Sending done.. \n", getpid());
            break;

        default:
            fprintf( stderr, "BE: pid : %d Unknown Protocol: %d\n", getpid(), tag );
            tag = PROT_EXIT;
            break;
        }

        fflush(stderr);

    } while( tag != PROT_EXIT );

    if( stream != NULL ) {
        while( ! stream->is_Closed() )
            sleep(1);

        delete stream;
    }

    // FE delete of the net will cause us to exit, wait for it
    net->waitfor_ShutDown();
    delete net;

    return 0;
}

size_t populateBuffer(char *sendAr, FILE* f ) {
    return fread(sendAr, 1, TOTAL_PACKET_SIZE, f);
}


