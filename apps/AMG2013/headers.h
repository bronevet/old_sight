#ifndef ROOT_HEADERS_H
#define ROOT_HEADERS_H

#include "sight.h"
#include "kulfi_structure.h"
using namespace sight;

#if defined(KULFI)
#define sightModularApp kulfiModularApp
#define sightModule     kulfiModule
#else
#define sightModularApp modularApp
#define sightModule     module
#endif

#endif
