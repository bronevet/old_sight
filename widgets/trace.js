var traceDataList = {};

var traceDataHash = {};
var traceLinkHash = {};

var minData = {};
var maxData = {};

// Records for each context key all the values ever observed for it
var allCtxtVals = {};

// Maps each trace/context key and value to its type:
// string - generic type elements of which are ordered lexically
// number
var traceValType = {};
var ctxtValType = {};

function isNumber(n) {
  return !isNaN(parseFloat(n)) && isFinite(n);
}

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

  // Update the info on the type of trace and context keys
  for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) {
    updateKeyValType(ctxtValType, ctxtKey, contextVals[ctxtKey]);
  } }
  for(traceKey in traceVals) { if(traceVals.hasOwnProperty(traceKey)) {
    updateKeyValType(traceValType, traceKey, traceVals[traceKey]);
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

// Given a mapping of trace/context keys to the types of their values,
// updates the mapping that, taking the next observation of the value into account 
function updateKeyValType(typemap, key, val) {
  if(isNumber(val)) {
    // If we don't yet know this key's type, initialize it to "number"
    if(!(key in typemap)) typemap[key]="number";
    // If we currently believe the key's type is something other than "number", set it to "string" since it includes all possibilities
    else if(typemap[key]!="number") typemap[key]="string";
  }
  // Since we don't know the type of this key, set it to "string" (overwrites any prior setting)
  typemap[val]="string";
}

// Given the type of a given key, returns an appropriate comparison function to use when sorting
// instances of the key
function getCompareFunc(keyType) {
  if(keyType == "number")
    return function(a, b) { return a-b; }
  else
    return function(a, b) { return a<b; }
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
      
      var ctxtKeys = [];
      for(ctxtKey in allCtxtVals) { if(allCtxtVals.hasOwnProperty(ctxtKey)) {
        ctxtKeys.push(ctxtKey);
      } }
      
      var numCols = 0;
      for(ctxt0Key in allCtxtVals[ctxtKeys[0]]) { if(allCtxtVals[ctxtKeys[0]].hasOwnProperty(ctxt0Key)) { numCols++; }}

      // Array of all the values of the first context key, in sorted order
      var ctxt0Keys = [];
      for(ctxt0Key in allCtxtVals[ctxtKeys[0]]) { if(allCtxtVals[ctxtKeys[0]].hasOwnProperty(ctxt0Key)) { ctxt0Keys.push(ctxt0Key); } }
      ctxt0Keys.sort(getCompareFunc(ctxtValType[ctxtKeys[0]]));

      // Array of all the values of the second context key, in sorted order
      var ctxt1Keys = [];
      for(ctxt1Key in allCtxtVals[ctxtKeys[1]]) { if(allCtxtVals[ctxtKeys[1]].hasOwnProperty(ctxt1Key)) { ctxt1Keys.push(ctxt1Key); } }
      ctxt1Keys.sort(getCompareFunc(ctxtValType[ctxtKeys[1]]));
      
      out += "<tr><td width="+(100/(numCols+1))+"%></td>\n";
      for(k0 in ctxt0Keys)
      { out += "<td width="+(100/(numCols+1))+"%>"+ctxt0Keys[k0]+"</td>\n"; }
      out += "</tr>";
      //alert(out);
      
      for(k1 in ctxt1Keys) {
        out += "<tr><td>"+ctxt1Keys[k1]+"</td>\n";
        for(k0 in ctxt0Keys) {
          var contextVals = {};
          contextVals[ctxtKeys[0]] = ctxt0Keys[k0];
          contextVals[ctxtKeys[1]] = ctxt1Keys[k1];
          var traceVals = getDataHash(traceDataHash[traceLabel], contextVals);
          var traceLinks = getDataHash(traceLinkHash[traceLabel], contextVals);
          if(traceVals) { 
            var valBucket = Math.floor((traceVals[traceAttrs[i]] - minData[traceAttrs[i]]) / valBucketSize);
            out += "<td bgcolor=\""+colors[Math.min(valBucket, colors.length-1)]+"\" ";
            if(traceLinks[traceAttrs[i]] != undefined && traceLinks[traceAttrs[i]] != "")
              out += "onclick=\""+traceLinks[traceAttrs[i]]+"\"";
            out += ">";
            //out += traceVals[traceAttrs[i]];
            out += "</td>\n";
          }
        }
        out += "</tr>\n";
      }
      
      out += "</table>";
    }
    alert(out);
    document.getElementById("div"+blockID).innerHTML=out;
  }
  
  displayTraceCalled = true;
}

