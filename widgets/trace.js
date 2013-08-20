var tracerData = [];
var tracerCols = [];

function traceRecord(traceVals, contextVals, viz) {
  if(tracerCols.length==0) {
    var ctxtCols = [];     
    for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) {
           if(viz == 'table')   ctxtCols.push({key:ctxtKey, label:ctxtKey, sortable:true});
      else if(viz == 'decTree') tracerCols.push(ctxtKey);
    } }
    
    var traceCols = [];
    for(traceKey in traceVals) { if(traceVals.hasOwnProperty(traceKey)) {
           if(viz == 'table')   traceCols.push({key:traceKey, label:traceKey, sortable:true});
    } }
    
    if(viz == 'table') 
      tracerCols = [ {label:"Context", children:ctxtCols},
                     {label:"Trace",   children:traceCols} ];
    else if(viz == 'decTree') 
    {}
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
  tracerData.push(allVals);
}

var displayTraceCalled = false;
function displayTrace(title, blockID, focusAttr, viz) {
  if(viz == 'table') {
    YUI().use("datatable-sort", function (Y) {
        // A table from data with keys that work fine as column names
        var traceTable = new Y.DataTable({
            columns: tracerCols,
            data   : tracerData,
            caption: title
        });
        
        traceTable.render("#div"+blockID);
      });
  } else {
    if(!displayTraceCalled) tracerData = _(tracerData);
    var model = id3(tracerData,focusAttr,tracerCols);
    drawGraph(model,"div"+blockID+":"+focusAttr);
  }
  
  displayTraceCalled = true;
}