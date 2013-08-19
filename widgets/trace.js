var tracerData = [];
var tracerCols = [];

function traceRecord(traceVals, contextVals) {
  var allVals = {};
  if(tracerCols.length==0) {
    var ctxtCols = [];     
    for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) {
      ctxtCols.push({key:ctxtKey, label:ctxtKey, sortable:true});
    } }
    
    var traceCols = [];
    for(traceKey in traceVals) { if(traceVals.hasOwnProperty(traceKey)) {
      traceCols.push({key:traceKey, label:traceKey, sortable:true});
    } }
    
    tracerCols = [ {label:"Context", children:ctxtCols},
                   {label:"Trace",   children:traceCols} ];
  }
  
  for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) {
    allVals[ctxtKey] = contextVals[ctxtKey];
  } }
  for(traceKey in traceVals) { if(traceVals.hasOwnProperty(traceKey)) {
    allVals[traceKey] = traceVals[traceKey];
  } }
  
  tracerData.push(allVals);
}

function displayTrace(title, blockID) {
  YUI().use("datatable-sort", function (Y) {
      // A table from data with keys that work fine as column names
      var traceTable = new Y.DataTable({
          columns: tracerCols,
          data   : tracerData,
          caption: title
      });
      
      traceTable.render("#div"+blockID);
    });
}