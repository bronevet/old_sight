//
// Created by Udayanga Wickramasinghe on 11/14/14.
// Copyright (c) 2014 CREST. All rights reserved.
//

#include "mrnet_testbase.h"
#include "test_utils.h"
#include "fdstream.h"

bool MRNetTestBase::setupMRNetFront() {
    //setup standard input
    setupMRNetFront(1);
    return true;
}

bool MRNetTestBase::setupMRNetFront(int be) {
    //setup standard input
    merge_output_descr = stdin;

    char* top = "../bin/top_file_s2";
    char* so = "../lib/libmrnet_filter.so";
    //spring up mrnet front end
    startup(top, so, NULL, 0, be);
    return true;
}

bool MRNetTestBase::setupMRNetBack() {
    char* mrnet_emitter_path = "smrnet_be";

    int parent_port = 2231;
    //setup outgoing stream for this dbgstream
    //smrnet_be will be the backend emitter that connects to FE
    char emitter_cmdline[1000];
    // Command line args are: binary parent_hostname, parent_port, parent_rank, my_hostname, my_rank
    snprintf(emitter_cmdline, 1000, "smrnet_be 127.0.0.1 %d 0 127.0.0.1 1000", parent_port);
    FILE *out = popen("smrnet_be", "w");
    if(out == NULL) assert(false);
    int outFD = fileno(out);
    dbgBuf* buf = new dbgBuf(new fdoutbuf(outFD));

    //use this out pointer for [dbg]
    dbg->buf = buf;


    //run sample application
    unsetenv("SIGHT_FILE_OUT");
    unsetenv("SIGHT_LAYOUT_EXEC");
    unsetenv("MRNET_MERGE_EXEC");
    TestUtils::run_sight_app();

    //finalize sight app
    AbortHandlerInstantiator::finalizeSight();

    return true;
}