var tracerData = {};
var tracerCols = {};

function traceRecord(traceLabel, traceVals, contextVals, viz) {
  // If this is the first time we've added a record to this trace
  if(!tracerData.hasOwnProperty(traceLabel)) {
    tracerData[traceLabel] = [];
  }
  
  var allVals = {};
  if(viz == 'table' || viz == 'decTree') {
    for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) {
      allVals[ctxtKey] = contextVals[ctxtKey];
    } }
    for(traceKey in traceVals) { if(traceVals.hasOwnProperty(traceKey)) {
      allVals[traceKey] = traceVals[traceKey];
    } }
  }
  tracerData[traceLabel].push(allVals);
}

var displayTraceCalled = {};
function displayTrace(traceLabel, blockID, contextAttrs, traceAttrs, viz) {
  if(viz == 'table') {
    var tracerCols = [];
    
    var ctxtCols = [];
    for(i in contextAttrs)
      ctxtCols.push({key:contextAttrs[i], label:contextAttrs[i], sortable:true});
    
    var traceCols = [];
    for(i in traceAttrs)
      traceCols.push({key:traceAttrs[i], label:traceAttrs[i], sortable:true});
    
    YUI().use("datatable-sort", function (Y) {
        // A table from data with keys that work fine as column names
        var traceTable = new Y.DataTable({
            columns: [ {label:"Context", children:ctxtCols},
                       {label:"Trace",   children:traceCols} ],
            data   : tracerData[traceLabel],
            caption: traceLabel
        });
        
        traceTable.render("#div"+blockID);
      });
  } else {
    //if(!displayTraceCalled.hasOwnProperty(traceLabel)) {
    tracerData[traceLabel] = _(tracerData[traceLabel]);
    var model = id3(tracerData[traceLabel], traceAttrs[0], contextAttrs);
    //alert(document.getElementById("div"+blockID).innerHTML)
    // Create a div in which to place this attribute's decision tree
    document.getElementById("div"+blockID).innerHTML += traceAttrs[0]+"<div id='div"+blockID+":"+traceAttrs[0]+"'></div>";
    drawGraph(model,"div"+blockID+":"+traceAttrs[0]);
  }
  
  displayTraceCalled = true;
}
