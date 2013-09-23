var traceDataList = {};
var traceDataHash = {};
var traceLinkHash = {};

var minData = {};
var maxData = {};

// Records all the values ever observed for each context dimension
var allCtxtVals = {};

function traceRecord(traceLabel, traceVals, traceValLinks, contextVals, viz) {
  // If this is the first time we've added a record to this trace
  if(!traceDataList.hasOwnProperty(traceLabel)) {
    traceDataList[traceLabel] = [];
    traceDataHash[traceLabel] = {};
    traceLinkHash[traceLabel] = {};
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
  if(viz == 'table' || viz == 'lines' || viz == 'decTree' || viz == 'heatmap') {
    
    // Add the data
    for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) {
      allVals[ctxtKey] = contextVals[ctxtKey];
      if(parseInt(minData[ctxtKey]) > parseInt(contextVals[ctxtKey])) minData[ctxtKey] = contextVals[ctxtKey];
      if(parseInt(maxData[ctxtKey]) < parseInt(contextVals[ctxtKey])) maxData[ctxtKey] = contextVals[ctxtKey];
    } }
    for(traceKey in traceVals) { if(traceVals.hasOwnProperty(traceKey)) {
      allVals[traceKey] = traceVals[traceKey];
      if(parseInt(minData[traceKey]) > parseInt(traceVals[traceKey])) minData[traceKey] = traceVals[traceKey];
      if(parseInt(maxData[traceKey]) < parseInt(traceVals[traceKey])) maxData[traceKey] = traceVals[traceKey];
    } }
  }
  
  if(viz == 'heatmap') {
    for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) {
      if(allCtxtVals[ctxtKey] == undefined) { allCtxtVals[ctxtKey] = {}; }
      allCtxtVals[ctxtKey][contextVals[ctxtKey]] = 1;
    } }
  }
  
  traceDataList[traceLabel].push(allVals);
  
  addDataHash(traceDataHash[traceLabel], traceVals, contextVals);
  addDataHash(traceLinkHash[traceLabel], traceValLinks, contextVals);
}

function addDataHash(dataHash, traceVals, contextVals) {
  // Set ctxt to be a sorted array of the contextVals keys
  var ctxt = [];
  for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) { ctxt.push(ctxtKey); } }
  ctxt.sort();
  
  var subTDH = dataHash;
  for(var i=0; i<ctxt.length; i++) {
    if(subTDH[contextVals[ctxt[i]]] == undefined) { subTDH[contextVals[ctxt[i]]]={}; }
    subTDH = subTDH[contextVals[ctxt[i]]];
  }
  subTDH["data"] = traceVals;
}

function getDataHash(dataHash, contextVals) {
  // Set ctxt to be a sorted array of the contextVals keys
  var ctxt = [];
  for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) { ctxt.push(ctxtKey); } }
  ctxt.sort();

  var subTDH = dataHash;  
  for(var i=0; i<ctxt.length; i++) {
    if(subTDH[contextVals[ctxt[i]]] == undefined) { return undefined; }
    subTDH = subTDH[contextVals[ctxt[i]]];
  }
  if(subTDH["data"] == undefined) { return undefined; }
  return subTDH["data"];
}

var displayTraceCalled = {};
function displayTrace(traceLabel, blockID, contextAttrs, traceAttrs, viz) {
  if(viz == 'table') {
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
            data   : traceDataList[traceLabel],
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
            dataProvider: traceDataList[traceLabel],
            categoryKey: contextAttrs[0],
            seriesKeys: traceAttrs,
            axes:myAxes,
            render: "#div"+blockID+"_"+contextAttrs[0]
        });
      });
  } else if(viz == 'decTree') {
    //if(!displayTraceCalled.hasOwnProperty(traceLabel)) {
    traceDataList[traceLabel] = _(traceDataList[traceLabel]);
    var model = id3(traceDataList[traceLabel], traceAttrs[0], contextAttrs);
    //alert(document.getElementById("div"+blockID).innerHTML)
    // Create a div in which to place this attribute's decision tree
    //document.getElementById("div"+blockID).innerHTML += traceAttrs[0]+"<div id='div"+blockID+":"+traceAttrs[0]+"'></div>";
    drawGraph(model,"div"+blockID+"_"+traceAttrs[0]);
  } else if(viz == 'heatmap') {
    var out = "";
    for(i in traceAttrs) { 
      out += traceAttrs[i]+"\n";
    
      var numColors = 100;
      var colors = gradientFactory.generate({
          from: "#0000FF",
          to: "#FF0000",
          stops: numColors
      })
    
      var valBucketSize = (maxData[traceAttrs[i]] - minData[traceAttrs[i]])/numColors;
      //out += "valBucketSize="+valBucketSize+", minData[traceAttrs[i]]="+minData[traceAttrs[i]]+", maxData[traceAttrs[i]="+maxData[traceAttrs[i]]+"<br>\n";
      out += "<table width=100%>";
      
      var traceKeys = [];
      for(traceKey in allCtxtVals) { if(allCtxtVals.hasOwnProperty(traceKey)) {
        traceKeys.push(traceKey);
      } }
      
      var numCols = 0;
      for(trace0K in allCtxtVals[traceKeys[0]]) { if(allCtxtVals[traceKeys[0]].hasOwnProperty(trace0K)) { numCols++; }}
      
      out += "<tr><td width="+(100/(numCols+1))+"%></td>\n";
      for(trace0K in allCtxtVals[traceKeys[0]]) { if(allCtxtVals[traceKeys[0]].hasOwnProperty(trace0K)) {
        out += "<td width="+(100/(numCols+1))+"%>"+trace0K+"</td>\n";
      } }
      out += "</tr>";
      //alert(out);
      
      for(trace1K in allCtxtVals[traceKeys[1]]) { if(allCtxtVals[traceKeys[1]].hasOwnProperty(trace1K)) {
        out += "<tr><td>"+trace1K+"</td>\n";
        for(trace0K in allCtxtVals[traceKeys[0]]) { if(allCtxtVals[traceKeys[0]].hasOwnProperty(trace0K)) {
          var contextVals = {};
          contextVals[traceKeys[0]] = trace0K;
          contextVals[traceKeys[1]] = trace1K;
          var traceVals = getDataHash(traceDataHash[traceLabel], contextVals);
          var traceLinks = getDataHash(traceLinkHash[traceLabel], contextVals);
          if(traceVals) { 
            var valBucket = Math.floor((traceVals[traceAttrs[i]] - minData[traceAttrs[i]]) / valBucketSize);
            out += "<td bgcolor=\""+colors[Math.min(valBucket, colors.length-1)]+"\" ";
            if(traceLinks[traceAttrs[i]] != undefined && traceLinks[traceAttrs[i]] != "")
              out += "onclick=\""+traceLinks[traceAttrs[i]]+"\"";
            out += "\">";
            //out += traceVals[traceAttrs[i]];
            out += "</td>\n";
          }
        } }
        out += "</tr>\n";
      } }
      
      out += "</table>";
    }
    //alert(out);
    document.getElementById("div"+blockID).innerHTML=out;
  }
  
  displayTraceCalled = true;
}
