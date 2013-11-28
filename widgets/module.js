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
