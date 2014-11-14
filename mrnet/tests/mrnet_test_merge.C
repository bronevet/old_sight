//
// Created by Udayanga Wickramasinghe on 11/14/14.
// Copyright (c) 2014 CREST. All rights reserved.
//

#include "mrnet_testbase.h"
#include "test_utils.h"
#include "fdstream.h"

class MRNetTestMerge : public MRNetTestBase{

void test(){
    setupMRNetFront();
    sleep(2);
    setupMRNetBack();
    sleep(2);

    //use merge output
    int fD = fileno(merge_output_descr);
    streambuf* buf = new streambuf(new fdoutbuf(fD));
    stringstream ss ;

    //TODO
    //check for comparison
    TestUtils::test_str_equals(ss.str(), "");
    //started up without error
    TestUtils::log_result("MRNetTestMerge", true);

    delete buf ;
}

};