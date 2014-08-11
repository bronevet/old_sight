// Records the source and target divs of all the arrows registered via 
// createUniqueMarkArrowFrom() and createUniqueMarkArrowTo().
// Maps the arrow's source blockID to a non-empty array of target uniqueIDs.
var parArrowsFrom = {};
// Maps the arrow's source div to a non-empty hash where the keys are the associated labels
//var parArrowsFromLabels = {};

// Maps the arrow's target blockID to a non-empty array of source uniqueIDs.
var parArrowsTo = {};
// Maps the arrow's target div to a non-empty hash where the keys are the associated labels
//var parArrowsToLabels = {};

// Maps each barrier unique ID to a non-empty array of the blokIDs associated with the barrier
var parBarriers = {};
// Maps each barrier ID to a non-empty hash where the keys are the labels associated with the barrier
//var parBarrierLabels = {};

// Maps the unique ID of each visualization object to a reference to it
var vizID2Obj = {};

// Maps each visualization object to a hash where the keys are blockIds that relate to it. This is important 
// for determining which objects should be shown based on which block divs are currently visible or hidden;
var vizObjects2BlockID = {};

function createUniqueMarkArrowFrom(sourceBlockID, targetUIDs)
{
  if(targetUIDs.length > 0) {
    // Read over all the uniqueMarks in targetUIDs and add the edges to parArrowsFrom
    
    // Map an empty array to sourceDiv since we're about to fill it with at least one targetDiv
    if(!(sourceBlockID in parArrowsFrom)) {
      parArrowsFrom[sourceBlockID]=[];
      //parArrowsFromLabels[sourceBlockID]={};
    }
  
    for(t in targetUIDs) parArrowsFrom[sourceBlockID].push(targetUIDs[t]);
    //for(l in labels) parArrowsFromLabel[sourceBlockID][labels[l]]=1;
  }
}

function createUniqueMarkArrowTo(targetBlockID, sourceUIDs)
{
  if(sourceUIDs.length > 0) {
    // Read over all the uniqueMarks in sourceUIDs and add the edges to parArrowsTo
    
    // Map an empty array to sourceDiv since we're about to fill it with at least one targetDiv
    if(!(targetBlockID in parArrowsFrom)) {
      parArrowsTo[targetBlockID]=[];
      //parArrowsToLabels[targetBlockID]={};
    }
  
    for(s in sourceUIDs) parArrowsTo[targetBlockID].push(sourceUIDs[s]);
    //for(l in labels) parArrowsToLabels[targetBlockID][labels[l]]=1;
  }
}

function createBarrierMember(barrierBlockID, barrierIDs)
{
  if(barrierIDs.length==1) {
    if(!(barrierIDs[0] in parBarriers)) {
      parBarriers[barrierIDs[0]]       = [];
      //parBarrierLabels[barrierIDs[0]] = {};
    }
  
    parBarriers[barrierIDs[0]].push(barrierBlockID);
    //for(l in barrierLabels) parBarrierLabels[barrierIDs[0]][barrierLabels[l]]=1;
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

// Concatenates the elements in the given array into a string, with the 
// different array elements separated with string sep
function concatenate(arr, sep) {
  var out="";
  if(arr !== undefined) {
    for(i in arr) {
      if(i==0) out += sep;
      out += arr[i];
    }
  }
  return out;
}

// Concatenates the keysin the given hash into a string, with the 
// different hash keys separated with string sep
function concatenateKeys(h, sep) {
  var out="";
  if(h !== undefined) {
    var i=0;
    for(key in h) { if(h.hasOwnProperty(key)) {
      if(i>0) out += sep;
      out += key;
      i++;
    } }
  }
  return out;
}


// The canvas on which we will draw arrows between causally-related events in different portions of the log
var parArrowCanvas;

// Maps the given function mapFunc(keyBlockID, valBlockID) to either parArrowsFrom/parArrowsFromLabels or 
// parArrowsTo/parArrowsToLabels, which are passed as the parArrows, parArrowsLabels arguments. 
// keyBlockID/valBlockID are the blockID of the center of the key div and the value
// div, respectively, in the map. For parArrowsFrom key=source, value=target and vice versal for parArrowsTo.
// mapFunc is only shown only if the key or value div is visible.
function mapParArrows(parArrows, mapFunc) {
  // Iterate over the arrow sources
  for(keyBlockID in parArrows) { if(parArrows.hasOwnProperty(keyBlockID)) {
    var keyDiv = document.getElementById("div"+keyBlockID);
    if(isHidden(keyDiv)) continue;
    // Iterate over this key's target uniqueIDs and find their associated divs
    
    // Hash that contains all the divs that are the mapped to IDs in targetUIDs
    var valueBlockIDs = {};
    for(t in parArrows[keyBlockID]) { if(parArrows[keyBlockID].hasOwnProperty(t)) {
      for(i in uniqueIDs2BlockID[parArrows[keyBlockID][t]]) { if(uniqueIDs2BlockID[parArrows[keyBlockID][t]].hasOwnProperty(i)) {
        var valueBlockID = uniqueIDs2BlockID[parArrows[keyBlockID][t]][i];
        valueBlockIDs[valueBlockID]++;
      } }
    } }
    
    // Hash that contains all the labels that are mapped 
    
    // Create arrows from keyDiv to each div in valueBlockIDs
    for(valueBlockID in valueBlockIDs) { if(valueBlockIDs.hasOwnProperty(valueBlockID)) {
      var valueDiv = document.getElementById("div"+valueBlockID);
      if(isHidden(valueDiv)) continue;
      if(valueDiv == null) continue;
      
      //console.log("["+keyCenter.x+", "+keyCenter.y+"] /"+keyBlockID+" => ["+valueCenter.x+", "+valueCenter.y+"]/"+valueBlockID);
      //mapFunc(keyBlockID, valueBlockID, concatenateKeys(parArrowsLabels[keyBlockID], "_"));
      /*var keyLabel = getUniqueIDLabelFromBlockID(keyBlockID, "_");
      var valueLabel = getUniqueIDLabelFromBlockID(valueBlockID, "_");
      var label="";
      if(keyLabel!="" && valueLabel!="") label = keyLabel+": "+valueLabel;
      else if(keyLabel!="")              label = keyLabel;
      else                               label = valueLabel;*/
      mapFunc(keyBlockID, valueBlockID, getUniqueIDLabelFromBlockID(keyBlockID, "_"), getUniqueIDLabelFromBlockID(valueBlockID, "_"));
    } }
  } }
}

function mapBarriers(mapFunc) {
  // Iterate over the arrow sources
  for(barUID in parBarriers) { if(parBarriers.hasOwnProperty(barUID)) {
    var centers = [];
    for(i in parBarriers[barUID]) { if(parBarriers[barUID].hasOwnProperty(i)) {
      var barDiv = document.getElementById("div"+parBarriers[barUID][i]);
      if(isHidden(barDiv)) continue;
      
      if(barDiv != null && !isHidden(barDiv))
        centers.push({blockID: parBarriers[barUID][i], center: getDivCenter(barDiv)});
    } }
    
    mapFunc(centers, getUniqueIDLabel(barUID, "_"));
  }}
}

// Adds the given blockID to the records for each of the visualization objects in the vizObjs array
function addVizObj2BlockID(blockID, vizObjs) {
  for(i in vizObjs) { if(vizObjs.hasOwnProperty(i)) {
    // Map the ID of this viz object to its reference
    vizID2Obj[vizObjs[i].id] = vizObjs[i];
    
    // Map the viz object's ID to blockID
    if(!(vizObjs[i] in vizObjects2BlockID)) vizObjects2BlockID[vizObjs[i].id] = {};
    vizObjects2BlockID[vizObjs[i].id][blockID] = 1;
  } }
}

var jsPlumbInstance;

// Called to draw arrows between causally-related events in different portions of the log
function showParallelArrows() {
/*  // Initialize the canvas
  if(parArrowCanvas === undefined) parArrowCanvas = createCanvasOverlay();
  // Grab a drawing context for the canvas
  var ctx = parArrowCanvas.canvas.getContext('2d');
  
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

  mapBarriers(function(centers) {
                // Find the left and right edge of the centers
                var left=999999, right=-1;
                for(i in centers) {
                  left  = Math.min(left,  centers[i].x);
                  right = Math.max(right, centers[i].x);
                }

                // Find the average y coordinate of the centers
                var avgY=0;
                for(i in centers) avgY += centers[i].y;
                avgY /= centers.length;

                ctx.beginPath();
                //console.log("["+left+","+avgY+"] - ["+right+","+avgY+"]");
                ctx.moveTo(left,  avgY);
                ctx.lineWidth = 5;
                ctx.lineTo(right, avgY);
                ctx.stroke();
              });
  
  // Redraw arrows whenever the window is resized
  //addEvent(window, "resize", refreshParallelArrows);
  addLazyWindowResizeEvent(refreshParallelArrows);*/
  
  jsPlumb.ready(function() {

	jsPlumbInstance = jsPlumb.getInstance({
		// default drag options
		DragOptions : { cursor: 'pointer', zIndex:2000 },
		/* // the overlays to decorate each connection with.  note that the label overlay uses a function to generate the label text; in this
		// case it returns the 'labelText' member that we set on each connection in the 'init' method below.
		ConnectionOverlays : [
			[ "Arrow", { location:1, } ],
			[ "Label", { 
				location:0.1,
				id:"label",
				cssClass:"aLabel"
			}]
		],*/
		Container:"detailContents"
	});

	// this is the paint style for the connecting lines..
	var connectorPaintStyle = {
		lineWidth:2,
		//strokeStyle:"#61B7CF",
		strokeStyle:"#000000",
		joinstyle:"round",
		//outlineColor:"white",
		//outlineColor:"#000000",
		//outlineWidth:2
	},
	// .. and this is the hover style. 
	connectorHoverStyle = {
		lineWidth:4,
		strokeStyle:"#216477",
		outlineWidth:2,
		outlineColor:"white"
	},
	endpointHoverStyle = {
		fillStyle:"#216477",
		strokeStyle:"#216477"
	},
	// the definition of source endpoints (the small blue ones)
	sourceEndpoint = {
		endpoint:"Blank",
		paintStyle:{ 
			strokeStyle:"#7AB02C",
			fillStyle:"transparent",
			radius:7,
			lineWidth:3 
		},				
		isSource:true,
		//connector:[ "Flowchart", { stub:[40, 60], gap:10, cornerRadius:5, alwaysRespectStubs:true } ],								                
		connector:[ "Straight", { stub:0, gap:0} ],
		connectorStyle:connectorPaintStyle,
		//hoverPaintStyle:endpointHoverStyle,
		//connectorHoverStyle:connectorHoverStyle,
    dragOptions:{},
    /*overlays:[
    	[ "Label", { 
        	location:[0.5, 1.5], 
        	label:"Drag",
        	cssClass:"endpointSourceLabel" 
        } ]
    ]*/
	},		
	// the definition of target endpoints (will appear when the user drags a connection) 
	targetEndpoint = {
		endpoint:"Blank",					
		paintStyle:{ fillStyle:"#7AB02C",radius:11 },
		hoverPaintStyle:endpointHoverStyle,
		maxConnections:-1,
		dropOptions:{ hoverClass:"hover", activeClass:"active" },
		isTarget:true,			
    /*overlays:[
    	[ "Label", { location:[0.5, -0.5], label:"Drop", cssClass:"endpointTargetLabel" } ]
    ]*/
	},	
  // The overlays to decorate each connection with
	DirArrowConnectionOverlays = [ "Arrow", { location:1, } ],
  UndirConnectionOverlays = [ "Diamond", { location:1, width:1} ],
			
	init = function(connection) {
		connection.getOverlay("label").setLabel(connection.sourceId.substring(15) + "-" + connection.targetId.substring(15));
		connection.bind("editCompleted", function(o) {
			if (typeof console != "undefined")
				console.log("connection edited. path is now ", o.path);
		});
	};			

	// suspend drawing and initialise.
	jsPlumbInstance.doWhileSuspended(function() {
	    mapParArrows(parArrowsTo, 
               function(targetBlockID, sourceBlockID, targetLabel, sourceLabel) {
                console.log(sourceBlockID+", "+targetBlockID);
                var from = jsPlumbInstance.addEndpoint("div"+sourceBlockID, sourceEndpoint, { anchor:"BottomCenter", uuid:sourceBlockID+"BottomCenter" });
                if(sourceLabel!="") from.addOverlay([ "Label", { id:"label", cssClass:"arrowParNodeLabel", label:sourceLabel}]);
                var to = jsPlumbInstance.addEndpoint("div"+targetBlockID, targetEndpoint, { anchor:"TopCenter", uuid:targetBlockID+"TopCenter" });
                if(targetLabel!="") to.addOverlay([ "Label", { id:"label", cssClass:"arrowParNodeLabel", label:targetLabel}]);
                var connection = jsPlumbInstance.connect({uuids:[sourceBlockID+"BottomCenter", targetBlockID+"TopCenter"], editable:false});
                //if(label != "") connection.addOverlay([ "Label", { location:Math.random(), id:"label", cssClass:"aLabel", label:label}]);
                connection.addOverlay(DirArrowConnectionOverlays);
                
                // Record the visualization objects mapped to the source/target blockIDs
                addVizObj2BlockID(sourceBlockID, [from, to/*, connection*/]);
                addVizObj2BlockID(targetBlockID, [from, to/*, connection*/]);
               });
               
       mapParArrows(parArrowsFrom, 
               function(sourceBlockID, targetBlockID, sourceLabel, targetLabel) {
                console.log(sourceBlockID+", "+targetBlockID);
                var from = jsPlumbInstance.addEndpoint("div"+sourceBlockID, sourceEndpoint, { anchor:"BottomCenter", uuid:sourceBlockID+"BottomCenter" });
                if(sourceLabel!="") from.addOverlay([ "Label", { id:"label", cssClass:"arrowParNodeLabel", label:sourceLabel}]);
                var to = jsPlumbInstance.addEndpoint("div"+targetBlockID, targetEndpoint, { anchor:"TopCenter", uuid:targetBlockID+"TopCenter" });
                if(targetLabel!="") to.addOverlay([ "Label", { id:"label", cssClass:"arrowParNodeLabel", label:targetLabel}]);
                var connection = jsPlumbInstance.connect({uuids:[sourceBlockID+"BottomCenter", targetBlockID+"TopCenter"], editable:false});
                //if(label != "") connection.addOverlay([ "Label", { location:Math.random(), id:"label", cssClass:"aLabel", label:label}]);
                connection.addOverlay(DirArrowConnectionOverlays);
                
                // Record the visualization objects mapped to the source/target blockIDs
                addVizObj2BlockID(sourceBlockID, [from, to/*, connection*/]);
                addVizObj2BlockID(targetBlockID, [from, to/*, connection*/]);
               });
               
       mapBarriers(function(barriers, label) {
                if(barriers.length<2) { alert("mapBarriers() ERROR: barriers has <2 divs"); }
                
                // Sort the barriers list according to the x coordinate
                barriers.sort(function(a, b){ return a.center.x-a.center.b; });
                
                // Connect the left-most to the right-most div
                var last = barriers.length-1;
                var end1 = jsPlumbInstance.addEndpoint("div"+barriers[0].blockID, sourceEndpoint, { anchor:"BottomCenter", uuid:"div"+barriers[0].blockID+"BottomCenter" });
                var end2 = jsPlumbInstance.addEndpoint("div"+barriers[last].blockID, targetEndpoint, { anchor:"TopCenter", uuid:"div"+barriers[last].blockID+"TopCenter" });
                var connection = jsPlumbInstance.connect({uuids:["div"+barriers[0].blockID+"BottomCenter", "div"+barriers[last].blockID+"TopCenter"], editable:false});
                connection.addOverlay([ "Label", { location:0.4+Math.random()/5, id:"label", labelStyle:{fillStyle:"#ffffff", color:"#000066", opacity:0.5}, cssClass:"barrierParConnectorLabel", label:label}]);
                connection.addOverlay(UndirConnectionOverlays);
                
                //addVizObj2BlockID(targetBlockID, [end1, end2, connection]);
               });

  	  // listen for new connections; initialise them the same way we initialise the connections at startup.
	    jsPlumbInstance.bind("connection", function(connInfo, originalEvent) { 
			init(connInfo.connection);
		});
	});

	jsPlumb.fire("jsPlumbDemoLoaded", jsPlumbInstance);
	
});

  addLazyWindowResizeEvent(refreshParallelArrows);
}

// Refreshes the positions of the arrows between causally-related events based on the current
// locations of their source/target divs
function refreshParallelArrows() {
  // Iterate over all the visualization objects created in showParallelArrows() and set their visibility status based on the visibility status
  for(vizID in vizObjects2BlockID) { if(vizObjects2BlockID.hasOwnProperty(vizID)) {
    // Iterate over all the blockIDs this object is associated with and check if they're all visible
    var isVisible=true;
    for(blockID in vizObjects2BlockID[vizID]) { if(vizObjects2BlockID[vizID].hasOwnProperty(blockID)); {
      var blockDiv = document.getElementById("div"+blockID);
      if(isHidden(blockDiv)) {
        isVisible=false;
        break;
      }
    }}
    
    if(!isVisible) {
      //vizID2Obj[vizID].addClass("hidden");
      //vizID2Obj[vizID].setVisible(isVisible);
      //jsPlumb.hide(vizID2Obj[vizID]);
      //jsPlumb.detachAllConnections(vizID2Obj[vizID]);
      vizID2Obj[vizID].setVisible(false);
      vizID2Obj[vizID].setEnabled(false);
      vizID2Obj[vizID].setStyle({visibility:"hidden"});
    }
  }}
  
  jsPlumbInstance.repaintEverything();
  addLazyWindowResizeEvent(refreshParallelArrows);
}

