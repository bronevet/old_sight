#ifndef SIGHT_API_HEADER
#define SIGHT_API_HEADER

//define Sight API below for C integration
//support for sight's main stream operations and widgets

#ifdef __cplusplus
extern "C" {
#endif

/***********************************
*      Initialization API          *
************************************/
//initializae sight with arguments
void SightInit(int argc, char** argv, char* title, char* workDir);


/***********************************
*      Stream funtions API         *
************************************/
void _dbg(char* str);
void _dbgprintf(const char * format, ... );


/***********************************
*      indent funtions API         *
************************************/


void* _indent();

/* returns an indent with string input
*/
void* _indent2(char* s);

/**
* release an indent
*/
void _indent_out(void* ind);


/***********************************
*      scope funtions API          *
************************************/

/**
* start a scope segment and return pointer to scope
*/
void* _scope(char* s);

typedef enum{
    low,
    medium,
    high,
    minimum
} scope_level;

/**
* start a scope segment with wieghted level
*/
void* _scope_l(char* s, scope_level l);
/**
* end a scope segment
*/
void _scope_out(void* scp);

#ifdef __cplusplus
}
#endif

#endif
