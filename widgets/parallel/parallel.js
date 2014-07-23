// Records the source and target divs of all the arrows registered via 
// createUniqueMarkArrowFrom() and createUniqueMarkArrowTo().
// Each key in parArrowsFrom is the arrow's source div and the value is a non-empty
// array of target uniqueIDs.
var parArrowsFrom = {};
// Each key in parArrowsTo is the arrow's target div and the value is a non-empty
// array of source uniqueIDs.
var parArrowsTo = {};

function createUniqueMarkArrowFrom(sourceBlockID, targetIDs)
{
  if(targetIDs.length > 0) {
    // Read over all the uniqueMarks in targetIDs and add the edges to parArrowsFrom
    
    // Map an empty array to sourceDiv since we're about to fill it with at least one targetDiv
    if(!(sourceBlockID in parArrowsFrom)) parArrowsFrom[sourceBlockID]=[];
  
    for(t in targetIDs) parArrowsFrom[sourceBlockID].push(targetIDs[t]);
  }
}

function createUniqueMarkArrowTo(targetBlockID, sourceIDs)
{
  if(sourceIDs.length > 0) {
    // Read over all the uniqueMarks in sourceIDs and add the edges to parArrowsTo
    
    // Map an empty array to sourceDiv since we're about to fill it with at least one targetDiv
    if(!(targetBlockID in parArrowsFrom)) parArrowsTo[targetBlockID]=[];
  
    for(s in sourceIDs) parArrowsTo[targetBlockID].push(sourceIDs[s]);
  }
}

function getPosition(element) {
    var xPosition = 0;
    var yPosition = 0;
  
    while(element && element!=document.body) {
        xPosition += (element.offsetLeft - element.scrollLeft + element.clientLeft);
        yPosition += (element.offsetTop - element.scrollTop + element.clientTop);
        element = element.offsetParent;
    }
    return { left: xPosition, top: yPosition };
}

function getDivCenter(div) {
  var offset = getPosition(div);
  return {x: (offset.left+div.offsetWidth/2),
          y: (offset.top+div.offsetHeight/2)};
}

// The canvas on which we will draw arrows between causally-related events in different portions of the log
var parArrowCanvas;

// Maps the given function mapFunc(keyCenter, valCenter) to either parArrowsFrom or parArrowsTo, which is passed
// as the parArrows argument. keyCenter/valCenter are the x/y coordinates of the center of the key div and the value
// div, respectively, in the map. For parArrowsFrom key=source, value=target and vice versal for parArrowsTo.
// If the key or value div is not visible, the x/y coordinates are negative.
function mapParArrows(parArrows, mapFunc) {
  // Iterate over the arrow sources
  for(keyBlockID in parArrows) { if(parArrows.hasOwnProperty(keyBlockID)) {
    var keyDiv = document.getElementById("div"+keyBlockID);
    // Iterate over this key's target uniqueIDs and find their associated divs
    
    // Hash that contains all the divs that are the mapped to IDs in targetIDs
    var valueBlockIDs = {};
    for(t in parArrows[keyBlockID]) {
      for(i in uniqueIDs2BlockID[parArrows[keyBlockID][t]]) {
        var valueBlockID = uniqueIDs2BlockID[parArrows[keyBlockID][t]][i];
        valueBlockIDs[valueBlockID]++;
      }
    }
    
    // Create arrows from keyDiv to each div in valueBlockIDs
    for(valueBlockID in valueBlockIDs) {
      var valueDiv = document.getElementById("div"+valueBlockID);
      if(valueDiv != null) {
        var keyCenter;
        if(!isHidden(keyDiv)) keyCenter = getDivCenter(keyDiv);
        else                  keyCenter = {x: -1, y:-1};
        if(!isHidden(valueDiv)) valueCenter = getDivCenter(valueDiv);
        else                    valueCenter = {x: -1, y:-1};
        
        //console.log("["+keyCenter.x+", "+keyCenter.y+"] /"+keyBlockID+" => ["+valueCenter.x+", "+valueCenter.y+"]/"+valueBlockID);
        mapFunc(keyCenter, valueCenter);
      }
    }
  }}
}

// Called to draw arrows between causally-related events in different portions of the log
function showParallelArrows() {
  // Initialize the canvas
  if(parArrowCanvas === undefined) parArrowCanvas = createCanvasOverlay();
  // Grab a drawing context for the canvas
  var ctx = parArrowCanvas.getContext('2d');
  
  mapParArrows(parArrowsFrom, 
               function(sourceCenter, targetCenter) {
                 if(sourceCenter.x>=0 && sourceCenter.y>=0 && targetCenter.x>=0 && targetCenter.y>=0)
                   drawArrow(ctx, sourceCenter.x, sourceCenter.y, targetCenter.x, targetCenter.y);
               });
               
  mapParArrows(parArrowsTo, 
               function(targetCenter, sourceCenter) {
                 if(sourceCenter.x>=0 && sourceCenter.y>=0 && targetCenter.x>=0 && targetCenter.y>=0)
                   drawArrow(ctx, sourceCenter.x, sourceCenter.y, targetCenter.x, targetCenter.y);
               });

  
  // Redraw arrows whenever the window is resized
  //addEvent(window, "resize", refreshParallelArrows);
  addLazyWindowResizeEvent(refreshParallelArrows);
}

// Refreshes the positions of the arrows between causally-related events based on the current
// locations of their source/target divs
function refreshParallelArrows() {
  console.log("refreshParallelArrows");
  // Only refresh if the canvas has been created
  if(parArrowCanvas !== undefined) { 
    // Grab a drawing context for the canvas
    var ctx = parArrowCanvas.getContext('2d');
    ctx.clearRect(0, 0, parArrowCanvas.width, parArrowCanvas.height);
    showParallelArrows();
  }
}
