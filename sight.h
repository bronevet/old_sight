// Copyright (c) 203 Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory
// Written by Greg Bronevetsky <bronevetsky1@llnl.gov>
//  
// LLNL-CODE-642002.
// All rights reserved.
//  
// This file is part of Sight. For details, see https://github.com/bronevet/sight. 
// Please read the COPYRIGHT file for Our Notice and
// for the BSD License.
#include "utils.h"
#include "sight_structure_internal.h"
#include "widgets/graph/graph_structure.h"
#include "widgets/scope/scope_structure.h"
#include "widgets/valSelector/valSelector_structure.h"
#include "widgets/trace/trace_structure.h"
#include "widgets/module/module_structure.h"
#include "widgets/source/source_structure.h"
#include "widgets/clock/clock_structure.h"
#include "widgets/hierGraph/hierGraph_structure.h"

using namespace sight::structure;

// Macro to make it easier to choose whether to enable or disable all instances of Sight objects and stream output in an application.
// Objects are declared as SIGHT(scope, s, ("scope label", scope::high"))
// Text output is emitted to stream sight: sight << "text"<<endl;
// All of these can be eliminated from the application to minimize the runtime cost of Sight via
// #define DISABLE_SIGHT
//
#ifdef DISABLE_SIGHT
#define SIGHT(varType, varName, varParams)
#define sght common::nullS
#define SightInit NullSightInit
#else
#define SIGHT(varType, varName, varParams) varType varName varParams
#define sght dbg
#endif


