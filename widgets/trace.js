var tracerData = {};
var tracerCols = {};

var minData = {};
var maxData = {};

function traceRecord(traceLabel, traceVals, contextVals, viz) {
  // If this is the first time we've added a record to this trace
  if(!tracerData.hasOwnProperty(traceLabel)) {
    tracerData[traceLabel] = [];
  }
  
  // Initialize the minimum and maximum for each key
  for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) {
    if(!minData.hasOwnProperty(ctxtKey)) minData[ctxtKey] = 1e100;
    if(!maxData.hasOwnProperty(ctxtKey)) maxData[ctxtKey] = -1e100;
  } }
  
  for(traceKey in traceVals) { if(traceVals.hasOwnProperty(traceKey)) {
    if(!minData.hasOwnProperty(traceKey)) minData[traceKey] = 1e100;
    if(!maxData.hasOwnProperty(traceKey)) maxData[traceKey] = -1e100;
  } }
  
  // Create an object that contains the data of the current observation
  var allVals = {};
  if(viz == 'table' || viz == 'lines' || viz == 'decTree') {
    
    // Add the data
    for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) {
      allVals[ctxtKey] = contextVals[ctxtKey];
      if(minData[ctxtKey] > contextVals[ctxtKey]) minData[ctxtKey] = contextVals[ctxtKey];
      if(maxData[ctxtKey] < contextVals[ctxtKey]) maxData[ctxtKey] = contextVals[ctxtKey];
    } }
    for(traceKey in traceVals) { if(traceVals.hasOwnProperty(traceKey)) {
      allVals[traceKey] = traceVals[traceKey];
      if(minData[traceKey] > traceVals[traceKey]) minData[traceKey] = traceVals[traceKey];
      if(maxData[traceKey] < traceVals[traceKey]) maxData[traceKey] = traceVals[traceKey];
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
  } else if(viz == 'lines') {
    var minVal=1e100, maxVal=-1e100;
    for(i in traceAttrs) { 
        if(minVal>minData[traceAttrs[i]]) 
          minVal=minData[traceAttrs[i]]; }
    for(o in traceAttrs) { 
        if(maxVal<minData[traceAttrs[i]]) 
          maxVal=maxData[traceAttrs[i]]; }
    
    // Create a div in which to place this context attribute's line graph 
    YUI().use("charts", function (Y) {
        var myAxes = {
            values:{
                keys:traceAttrs,
                type:"numeric",
                minimum:minVal,
                maximum:maxVal
            }
        };
      
        // A table from data with keys that work fine as column names
        var traceChart = new Y.Chart({
            type: "line",
            dataProvider: tracerData[traceLabel],
            categoryKey: contextAttrs[0],
            seriesKeys: traceAttrs,
            axes:myAxes,
            render: "#div"+blockID+"_"+contextAttrs[0]
        });
      });
  } else if(viz == 'decTree') {
    //if(!displayTraceCalled.hasOwnProperty(traceLabel)) {
    tracerData[traceLabel] = _(tracerData[traceLabel]);
    var model = id3(tracerData[traceLabel], traceAttrs[0], contextAttrs);
    //alert(document.getElementById("div"+blockID).innerHTML)
    // Create a div in which to place this attribute's decision tree
    //document.getElementById("div"+blockID).innerHTML += traceAttrs[0]+"<div id='div"+blockID+":"+traceAttrs[0]+"'></div>";
    drawGraph(model,"div"+blockID+"_"+traceAttrs[0]);
  }
  
  displayTraceCalled = true;
}
