// Copyright (c) 203 Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory
// Written by Greg Bronevetsky <bronevetsky1@llnl.gov>
//  
// LLNL-CODE-642002.
// All rights reserved.
//  
// This file is part of Sight. For details, see https://github.com/bronevet/sight. 
// Please read the COPYRIGHT file for Our Notice and
// for the BSD License.
#include <stdio.h>
#include <netdb.h>

int main (int argc, char *argv[]) {
    struct hostent *hstnm;
    if (argc != 2) {
        fprintf(stderr, "usage: %s hostname\n", argv[0]);
        return 1;
    }
    hstnm = gethostbyname (argv[1]);
    if (!hstnm)
        return 1;
    printf ("Name: %s\n", hstnm->h_name);
    return 0;
}
