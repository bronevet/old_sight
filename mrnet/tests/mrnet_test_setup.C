//
// Created by Udayanga Wickramasinghe on 11/14/14.
// Copyright (c) 2014 CREST. All rights reserved.
//

#include "mrnet_testbase.h"
#include "test_utils.h"


class MRNetTestSetup : public MRNetTestBase{

void test(){
    setupMRNetFront();
    sleep(2);
    setupMRNetBack();
    sleep(2);
    //started up without error
    TestUtils::log_result("MRNetTestSetup", true);
}

};