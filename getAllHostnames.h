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
#include <list>
#include <string>

// Returns true is some service is current listerning on the given port and false otherwise
bool isPortUsed(int port);

std::list<std::string> getAllHostnames();
