//
// Created by Udayanga Wickramasinghe on 11/14/14.
// Copyright (c) 2014 CREST. All rights reserved.
//

#include "mrnet_front.h"


class MRNetTestBase{
public:
    bool setupMRNetFront();
    bool setupMRNetFront(int be);
    bool setupMRNetBack();
    bool setup(){
        return setupMRNetFront() && setupMRNetBack();
    }

    virtual void test() = 0 ;
};

class MRNetTestMerge : public MRNetTestBase{
    void test();
}  ;

class MRNetTestSetup : public MRNetTestBase{
    void test();
} ;

class MRNetTestParserUnit : public MRNetTestBase{
    void test();
} ;