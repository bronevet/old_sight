var tracerData = [];
var tracerCols = [];

function traceRecord(key, val, contextVals) {
  if(tracerCols.length==0) {
    for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) {
      tracerCols.push(ctxtKey);
    } }
    tracerCols.push(key);
  }
  
  contextVals[key] = val;
  tracerData.push(contextVals);
}

function displayTrace(title, blockID) {
  YUI().use("datatable", function (Y) {
      // A table from data with keys that work fine as column names
      var traceTable = new Y.DataTable({
          columns: tracerCols,
          data   : tracerData,
          summary: title
      });
      
      traceTable.render("#div"+blockID);
    });
}