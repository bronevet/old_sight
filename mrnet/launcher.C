//
//  main.cpp
//  MPI_LAUNCH
//
//  Created by Udayanga Wickrmasinghe on 10/6/14.
//  Copyright (c) 2014 CREST. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h> 
#include <sstream>
#include <string.h>
#include <mpi.h>
#include <unistd.h>
#include "mrnet_front.h"
using namespace std;

char *write_topology_file(char **hostnames, int comm_nodes, int be_nodes, char *dir);
void read_app_params(char** buffer, char* params_file, int num_be_nodes);

char topfile[80];
char config_dir[200];
char structure_file[200];

static const int MAX_BUFFER_LENGTH = 200;

int main(int argc, char **argv) {

    int myid, numprocs;
    MPI_Status status;
    int nametag = 1000;
    int parnametag = 1001;
    int parporttag = 1002;
    int parranktag = 1003;

    if( (argc != 3) &&  (argc != 4) ){
        fprintf(stderr, "Usage: %s <config dir> <so_file path> [num backends]\n", argv[0]);
        exit(-1);
    }

    char* so_file_path = argv[2];
    char* conf_dir_path = argv[1];
    //create structure file path
    snprintf(structure_file, sizeof(structure_file), "%s/merged.structure",conf_dir_path);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    char hostname[MAX_BUFFER_LENGTH];
    int ret = gethostname(hostname, sizeof(hostname));
    if (ret == -1) {
        fprintf(stderr, "could not retrieve hostname from this node. hostname: %s\n", hostname);
        MPI_Abort(MPI_COMM_WORLD, 1);
        return -1;
    }

    int num_backends = numprocs - 1;
    if (argc == 4) {
        num_backends = atoi(argv[3]);
    }
    int comm_nodes = (numprocs - 1) - num_backends;

    //only for root process
    char **hostname_ar = NULL;
    char **app_params = NULL;
    if (myid == 0) {
//        hostname_ar = (char*)malloc(sizeof(char) * numprocs * 100);
        hostname_ar = (char **) malloc(sizeof(char *) * numprocs);
        int i = 0;
        for (i = 0; i < numprocs; i++) {
            hostname_ar[i] = (char *) malloc(sizeof(char) * MAX_BUFFER_LENGTH);
        }

        //gather all hostnames
        hostname_ar[0] = hostname;
        /* Receive messages with hostname*/
        /* from all other processes */
        for (i = 1; i < numprocs; i++) {
            MPI_Recv(hostname_ar[i], MAX_BUFFER_LENGTH, MPI_CHAR, i, nametag, MPI_COMM_WORLD, &status);
        }

        for (i = 0; i < numprocs; i++) {
            printf("hostname : %d  host : %s\n", i, hostname_ar[i]);
        }

        write_topology_file(hostname_ar, comm_nodes, num_backends, conf_dir_path);
        //"Usage: %s <topology file> <so_file> <num BEs> [optional - <connection file>]\n", argv[0])
        //i am the master start the MRNet FE
        //create topology file by connecting with slave nodes
        //recv all hostnames of connecting nodes

        //num_be - no of BE/app nodes
        //N - num_be - 1 ==> no of intermediate level comm nodes
        if (num_backends > (numprocs - 1)) {
            fprintf(stderr, "no of BE nodes %d exceeds number of available nodes %d \n", num_backends, numprocs - 1);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return -1;
        }

        startup(topfile, so_file_path, structure_file, comm_nodes, num_backends);
        //send BE nodes the above info
        //selct end portion of ranks [k , k+1 , k+2 ....... n-1 n] as BE application nodes
        //send BE with parameters - do this in Leafinfo rountine
//        char *parHostname = "parent.host";
//        char *par_port = "1000222";
//        char *par_rank = "0";
//        for(i = comm_nodes + 1 ; i < numprocs ; i++){
//            MPI_Send(parHostname, MAX_BUFFER_LENGTH, MPI_CHAR, i, parnametag, MPI_COMM_WORLD);
//            MPI_Send(par_port, MAX_BUFFER_LENGTH, MPI_CHAR, i, parporttag, MPI_COMM_WORLD);
//            MPI_Send(par_rank, MAX_BUFFER_LENGTH, MPI_CHAR, i, parranktag, MPI_COMM_WORLD);
//        }

    } else {
        /* Send own name back to process 0 */
        MPI_Send(hostname, MAX_BUFFER_LENGTH, MPI_CHAR, 0, nametag, MPI_COMM_WORLD);
        //comm nodes and BE nodes belong to this category

        //if this is a BE node --> recv parent port, parent rank and hostname
        /* Receive messages with hostname*/
        /* from all other processes */
        if (myid > comm_nodes) {
            char parHostname[MAX_BUFFER_LENGTH], par_port[MAX_BUFFER_LENGTH], par_rank[MAX_BUFFER_LENGTH];

            MPI_Recv(&parHostname, MAX_BUFFER_LENGTH, MPI_CHAR, 0, parnametag, MPI_COMM_WORLD, &status);
            MPI_Recv(&par_port, MAX_BUFFER_LENGTH, MPI_CHAR, 0, parporttag, MPI_COMM_WORLD, &status);
            MPI_Recv(&par_rank, MAX_BUFFER_LENGTH, MPI_CHAR, 0, parranktag, MPI_COMM_WORLD, &status);

            std::stringstream ss;
            ss << (1500 + myid);
            char* myrank = (char*) ss.str().c_str();
            //set ENV variable required for BE mrnet connection
            //MRNET_MERGE_EXEC
            char env_var[350];
            snprintf(env_var, sizeof(env_var), "%s  %s %s %s %s %s","smrnet_be", parHostname, par_port, par_rank,
                    hostname, myrank );
            setenv("MRNET_MERGE_EXEC", env_var, 1);
            unsetenv("SIGHT_FILE_OUT");

            //init app parameters
            int i = 0 ;
            app_params = (char **) malloc(sizeof(char*) * num_backends);
            for (i = 0; i < num_backends; i++) {
                app_params[i] = (char *) malloc(sizeof(char) * MAX_BUFFER_LENGTH);
            }

            snprintf(config_dir, sizeof(config_dir), "%s/app.params", conf_dir_path);
            read_app_params(app_params, config_dir, num_backends);

            for (i = 0; i < num_backends; i++) {
                printf("params : %d  p : %s\n", i, app_params[i]);
            }

            //execute BE application
//            char* app = "/Users/usw/Install/Appcode/MPI_LAUNCH/MPI_LAUNCH/MPI_LAUNCH/a.out";
            char cmd[350];
//            snprintf(cmd, sizeof(cmd), "%s  %s %s %s %s %s",app, parHostname, par_port, par_rank, hostname, myrank );
//            snprintf(cmd, sizeof(cmd), "%s  %s %s %s %s %s",app, parHostname, par_port, par_rank, hostname, myrank );
            int my_index = myid - comm_nodes - 1 ;
            snprintf(cmd, sizeof(cmd), "%s", app_params[my_index] );
            system(cmd);
            printf("be_node : %d recieved parent parameters : %s : %s : %s  total BEs : %d \n",myid, parHostname,
                    par_port, par_rank, num_backends);

        }


    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}

int main_test1() {
    char **hostname_ar = NULL;
    hostname_ar = (char **) malloc(sizeof(char *) * 5);
    int i = 0;

    for (i = 0; i < 5; i++) {
        char host[20] = "host.";
        hostname_ar[i] = (char *) malloc(sizeof(char) * MAX_BUFFER_LENGTH);
//        printf("%s\n", host.c_str());
        strcpy(hostname_ar[i], host); //)hostname_ar[i] =  "aaaaa";
    }

    char *top = write_topology_file(hostname_ar, 0, 2, "/Users/usw/Install/Appcode/MPI_LAUNCH/MPI_LAUNCH/MPI_LAUNCH");
    printf("%s \n", top);
    return 0;
}

char *write_topology_file(char **hostnames, int comm_nodes, int be_nodes, char *dir) {
    snprintf(topfile, sizeof(topfile), "%s/top_file", dir);
    //handle root
    int i = 0, k = 0;
    //if no comm nodes there are just 2 levels
    //wrtie root node thats it
    if (comm_nodes == 0) {
        FILE *top_File = fopen(topfile, "w");
        fprintf(top_File, "%s:%d ;\n", hostnames[0], 0);
        fflush(top_File);
        fclose(top_File);
    } else {
        FILE *top_File = fopen(topfile, "w");
        fprintf(top_File, "%s:%d => \n", hostnames[0], 0);

        //for each comm_node include in topology file
        for (i = 1; i <= comm_nodes; i++) {
            fprintf(top_File, "\t%s:%d \n", hostnames[i], i);
        }
        fflush(top_File);
        fclose(top_File);
    }

    return topfile;
}

void read_app_params(char** buffer, char* params_file, int num_be_nodes){
    FILE *param_File_Desc = fopen(params_file, "r");
    char line[MAX_BUFFER_LENGTH];
    int i = 0 ;
    while ( fgets (line , sizeof(line) , param_File_Desc) != NULL ){
        if(i == num_be_nodes){
            break;
        }
        strcpy(buffer[i], line);
        i++;
    }

    if(i < num_be_nodes){
        fprintf(stderr, "Invalid [app].params file in %s no of BE nodes %d does not match the available parameters \n",
                params_file, num_be_nodes);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

}
