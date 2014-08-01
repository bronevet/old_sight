// Hash that maps unique IDs to the array of ID of the blocks they are mapped to
var uniqueIDs2BlockID={};

function registerUniqueMark(blockID, allIDs) {
  for(var ID in allIDs) {
    if(allIDs[ID] in uniqueIDs2BlockID)
      uniqueIDs2BlockID[allIDs[ID]].push(blockID);
    else
      uniqueIDs2BlockID[allIDs[ID]] = [blockID];
  }
}
