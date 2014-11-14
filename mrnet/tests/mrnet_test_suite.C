//
// Created by Udayanga Wickramasinghe on 11/14/14.
// Copyright (c) 2014 CREST. All rights reserved.
//

#include "mrnet_testbase.h"
#include "test_utils.h"
#include "fdstream.h"

int main(){
    vector<MRNetTestBase> testcases;

    TestUtils::log_result("Starting Test Suite", true);

    //list all the test cases
    MRNetTestSetup t1 ;
    testcases.push_back(t1);
    MRNetTestMerge t2;
    testcases.push_back(t2);
    MRNetTestMerge t3;
    testcases.push_back(t3);


    vector<MRNetTestBase>::iterator it;
    //once we have all test cases test them seqeunetially
    for(it = testcases.begin() ; it != testcases.end() ; it++){
        //test case
        MRNetTestBase test_case = *it;
        test_case.test();

        //sleep for few hundred micro seconds
        usleep(200);
    }


    TestUtils::log_result("Ending Test Suite", true);

    return 0;
}