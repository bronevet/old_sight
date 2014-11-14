//
// Created by Udayanga Wickramasinghe on 11/14/14.
// Copyright (c) 2014 CREST. All rights reserved.
//
#include "test_utils.h"


void TestUtils::log_result(char* label, bool r){
    if(r){
        printf("test [%s] Success !!", label);
    }else{
        printf("test [%s] Failure !!", label);
    }
}


void TestUtils::test_true(bool arg1, bool arg2){

    assert(arg1 == arg2);
}

void TestUtils::test_equals(char arg1, char arg2){
    assert(arg1 == arg2);
}

void TestUtils::test_str_equals(char* arg1, char* arg2){
    bool isEqv = true ;
    while(arg1 != '\0'){
        if(arg1 != arg2){
            assert(false);
        }
        arg1++;
        arg2++;
    }

    //if all characters matched and end is reached on both strings
    //then we have a perfectly matching string
    if(isEqv && arg1=='\0' && arg2=='\0'){
        assert(true);
    }else{
        assert(false);
    }

}

/**
* simple application routine
*/
void TestUtils::run_sight_app(){
    char* argv[] = { {"test"}};
    SightInit(1, argv, "1.Testsample", "dbg.1.TestSample");

    dbg << "<h1>Example 1: TestSample</h1>" << endl;
    {
        indent indA;
        int i = 0 ;
        for ( i = 0; i < 5; ++i) {
            dbg << "This text was indented."<<endl;
        }
    }
    dbg << "No indentation here"<<endl;

    // Any text can be used for indentation
    indent indB("###");
    dbg << "All subsequent text will have hashes prepended"<<endl;
    dbg << "Such as here\n";
    dbg << "And here"<<endl;


}
