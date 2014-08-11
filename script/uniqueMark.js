// Hash that maps unique IDs to the array of ID of the blocks they are mapped to
var uniqueIDs2BlockID={};

// Hash that maps each uniqueBlockID to the hash where the keys are the labels associated with the unique blockID
var uniqueBlockIDs2Labels={};

// Hash that maps each blockID to a hash where the keys are the unique IDs mapped to it
var blockID2UniqueIDs={};

function registerUniqueMark(blockID, allUIDs, allLabels) {
  // Map each unique ID in allUIDs to blockID and each label in allLabels
  for(var ID in allUIDs) { if(allUIDs.hasOwnProperty(ID)) {
    if(!(allUIDs[ID] in uniqueIDs2BlockID)) {
      uniqueIDs2BlockID[allUIDs[ID]] = [];
      uniqueBlockIDs2Labels[allUIDs[ID]] = {};
    }
    
    uniqueIDs2BlockID[allUIDs[ID]].push(blockID);
    for(label in allLabels) { if(allLabels.hasOwnProperty(label)) {
      uniqueBlockIDs2Labels[allUIDs[ID]][allLabels[label]]=1;
    } }
  } }
  
  // Map blockID to all unique IDs in allUIDs
  if(!(blockID in blockID2UniqueIDs)) blockID2UniqueIDs[blockID] = {};
  for(var ID in allUIDs) { if(allUIDs.hasOwnProperty(ID)) {
     blockID2UniqueIDs[blockID][allUIDs[ID]] = 1;
  } }
}

// Given a blockID returns a concatenation of the labels associated with all the uniqueIDs mapped to this blockID,
// with the individual labels separated by string sep.
function getUniqueIDLabelFromBlockID(blockID, sep) {
  var fullLabel="";
  for(uniqueID in blockID2UniqueIDs[blockID]) { if(blockID2UniqueIDs[blockID].hasOwnProperty(uniqueID)) {
    for(label in uniqueBlockIDs2Labels[uniqueID]) { if(uniqueBlockIDs2Labels[uniqueID].hasOwnProperty(label)) {
      if(fullLabel != "") fullLabel += sep;
      fullLabel += label;
    }}
  }}
  
  return fullLabel;
}

// Given a uniqueID returns a concatenation of the labels associated with it, with the individual labels separated by string sep.
function getUniqueIDLabel(uniqueID, sep) {
  var fullLabel="";
  for(label in uniqueBlockIDs2Labels[uniqueID]) { if(uniqueBlockIDs2Labels[uniqueID].hasOwnProperty(label)) {
    if(fullLabel != "") fullLabel += sep;
    fullLabel += label;
  }}
  
  return fullLabel;
}
