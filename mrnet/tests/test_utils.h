//
// Created by Udayanga Wickramasinghe on 11/14/14.
// Copyright (c) 2014 CREST. All rights reserved.
//
#include "assert.h"
#include "stdio.h"
#include "sight.h"
#include <map>
#include <assert.h>
using namespace std;
using namespace sight;


class TestUtils {
public:

    static void log_result(char*, bool);

    static void test_true(bool arg1, bool arg2);

    static void test_equals(char arg1, char arg2);

    static void test_str_equals(char *arg1, char *arg2);

    static void run_sight_app();
};
