//
//  main.cpp
//  ExampleLinkC_Cplus
//
//  Created by Udayanga Wickramasinghe on 2/11/15.
//  Copyright (c) 2015 crest. All rights reserved.
//

#include "sight_api.h"
#include "sight.h"
#include <stdio.h>
#include <stdarg.h>

using namespace std;
using namespace sight;

void SightInit(int argc, char** argv, char* title, char* workDir){
    sight::structure::SightInit(argc, argv, title, workDir);
}

void _dbg(char* str){
    dbg << str << endl;
}

void _dbgprintf(const char * format, ... ){
  va_list args;
  va_start(args, format);
  char printbuf[100000];
  vsnprintf(printbuf, 100000, format, args);
  va_end(args);

  dbg << printbuf << endl;
}

void* _indent(){
    indent* _ind = new indent();
    return _ind;
}

void* _indent2(char* s){
    indent* _ind = new indent(s);
    return _ind;
}

void _indent_out(void* ind){
    indent* _ind = (indent*) ind ;
    //explicitly makes indent out of scope
    delete _ind ;
}

void* _scope(char* s){
    scope* _scope = new scope(s);
    return _scope;
}

void* _scope_l(char* s, scope_level l){
    scope* _scope;
    switch (l){
        case low:
            _scope = new scope(s, scope::low);
            break;
        case  medium:
            _scope = new scope(s, scope::medium);
            break;
        case high:
            _scope = new scope(s, scope::high);
            break;
        case minimum:
            _scope = new scope(s, scope::minimum);
            break;
        default:
            _scope = NULL;
    }

    return _scope;
}

void _scope_out(void* scp){
    scope* _scope = (scope*) scp ;
    delete _scope ;
}

