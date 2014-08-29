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
var buttonCommands = {};
function registerModuleButtonCmd(buttonID, command) {
  buttonCommands[buttonID] = command;
}

function ClickOnModuleNode(label, target, buttonID) {
  //alert(label+": "+target+" buttonID="+buttonID);
  if(buttonID>=0) 
    eval(buttonCommands[buttonID]);
  return false;
}
