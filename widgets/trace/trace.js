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
// Maps each context/trace key to a unique numeric ID suitable for indexing into a dense array
var ctxtKey2ID = {};
var traceKey2ID = {};
// Array of all context/trace keys, at the indexes specified in the above hashes
var ctxtKeys = {};
var traceKeys = {};

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
  
  // Update the mapping from trace and context key names to their unique IDs
  for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) {
    if(ctxtKey2ID[traceLabel] == undefined) {
      ctxtKey2ID[traceLabel] = {};
      ctxtKeys[traceLabel] = [];
    }
    if(ctxtKey2ID[traceLabel][ctxtKey] == undefined) {
      ctxtKey2ID[traceLabel][ctxtKey] = Object.keys(ctxtKey2ID[traceLabel]).length;
      ctxtKeys[traceLabel].push(ctxtKey);
  } } }
  for(traceKey in traceVals) { if(traceVals.hasOwnProperty(traceKey)) {
    if(traceKey2ID[traceLabel] == undefined) {
      traceKey2ID[traceLabel] = {};
      traceKeys[traceLabel] = [];
    }
    if(traceKey2ID[traceLabel][traceKey] == undefined) {
      traceKey2ID[traceLabel][traceKey] = Object.keys(traceKey2ID[traceLabel]).length;
      traceKeys[traceLabel].push(traceKey);
  } } }
  
  // Create an object that contains the data of the current observation
  var allVals = {};
  if(viz == 'table' || viz == 'lines' || viz == 'decTree' || viz == 'heatmap' || viz == 'boxplot') {
    // Add the data
    for(ctxtKey in contextVals) { if(contextVals.hasOwnProperty(ctxtKey)) {
      allVals[ctxtKey] = contextVals[ctxtKey];
      if(parseFloat(minData[ctxtKey]) > parseFloat(contextVals[ctxtKey])) minData[ctxtKey] = contextVals[ctxtKey];
      if(parseFloat(maxData[ctxtKey]) < parseFloat(contextVals[ctxtKey])) maxData[ctxtKey] = contextVals[ctxtKey];
    } }
    for(traceKey in traceVals) { if(traceVals.hasOwnProperty(traceKey)) {
      allVals[traceKey] = traceVals[traceKey];
      if(parseFloat(minData[traceKey]) > parseFloat(traceVals[traceKey])) minData[traceKey] = traceVals[traceKey];
      if(parseFloat(maxData[traceKey]) < parseFloat(traceVals[traceKey])) maxData[traceKey] = traceVals[traceKey];
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
// traceLabel: the string label of the trace that needs to be displayed.
// We provide to ways to organize information based on the values of context attributes: split and project, each 
//       of which apply to a subset of the context attributes.
//   Split: We split the observations according to the values of the context attributes and 
//   show the subset of observations that share the same values of the context attributes separately.
//   Project: We show all the data as a function of the context attributes.
//   We ignore the differences among the remaining attributes. Thus, if we have attributes a0, a1, a2, where
//   a0 is split and a1 is projected, we break up the observations according to the values that a0 took on in
//   each observation. Within each such split we plot all the observations as a function of their value for the
//   a1 attribute and do not differentiate according to the value of the a2 attribute.
// splitCtxtAttrs, projectCtxtAttrs: Lists of attribute names to treat according to each organization policy.
// splitCtxtHostDivs: list of hashes that map each combination of keys in splitCtxtAttrs and their values to the 
//    ID of the div where their visualization will be displayed in format:
//    [{ctxt:{key0:..., val0:..., key1:..., val1:..., ...}, div:divID}, ...]
// traceAttrs: The names of the trace attributes, the observed values of which are displayed.
// hostDivID: ID of the div where the visualizations for each combination of values of the split context keys that
//    is not specified in splitCtxtHostDivs will be placed.
// viz: name of the visualization type to be used
// loc: location where the visualization will be shown relative to the trace: "showBegin" or "showEnd"
// showFresh: boolean that indicates whether we should overwrite the prior contents of hostDiv (true) or whether we should append
//      to them (false)
// showLabels: boolean that indicates whether we should show a label that annotates a data plot (true) or whether
//      we should just show the plot (false)
/*function displayTrace(traceLabel, splitCtxtAttrs, projectCtxtAttrs, traceAttrs, hostDivID, splitCtxtHostDivs, viz, loc) {
  var numSplitContextAttrs=0;
  for(i in splitCtxtAttrs)   { if(splitCtxtAttrs.hasOwnProperty(i))   { numSplitContextAttrs++; } }
  
  // If no split was requested, show all the data in hostDivID
  if(numSplitContextAttrs==0)
    displayTraceProjection(traceLabel, traceDataList[traceLabel], hostDivID, projectCtxtAttrs, traceAttrs, viz, loc); 
  else {
    // Hash that maps the keys in splitCtxtAttrs to true for faster membership lookups
    var splitCtxtAttrsHash = {};
    for(k in splitCtxtAttrs) { if(splitCtxtAttrs.hasOwnProperty(k)) {
      splitCtxtAttrsHash[k] = true;
    } }
    
    // Multi-level hashtable that maps the various observed combinations of values for the split context keys
    // to just the observations that have these values for these keys
    var splitTDL = new HTNode();
    for(i in traceDataList[traceLabel]) { if(traceDataList[traceLabel].hasOwnProperty(i)) {
      // Records the values of the split context keys for observation i
      var splitCtxtKeyVals = {};
      for(k in traceDataList[traceLabel][i]) { if(traceDataList[traceLabel][i].hasOwnProperty(k)) {
        // If this key is a member of splitCtxtAttrs, record its value in splitCtxtKeyVals
        splitCtxtKeyVals[k] = traceDataList[traceLabel][i][k];
      } }
      
      // Convert splitCtxtKeyVals into an array of values for keys in splitCtxtAttrs, sorted in the same order 
      // as the keys in splitCtxtAttrs
      var splitValsArray = [];
      for(k in splitCtxtAttrs) { if(splitCtxtAttrs.hasOwnProperty(k)) {
        splitCtxtAttrs.push(splitCtxtKeyVals[k]);
      } }
      
      // Add the current observation to the sublist dedicated to observations with the same values for the split context keys
      splitTDL = pushHash(splitTDL, splitValsArray, "obs", traceDataList[traceLabel][i]);
    } }
    
    // Turn splitCtxtHostDivs into a hashtable with the values of the split context as keys
    var splitDivs = new HTNode();
    for(i in splitCtxtHostDivs) { if(splitCtxtHostDivs.hasOwnProperty(i)) {
      var splitValsArray = [];
      for(k in splitCtxtAttrs) { if(splitCtxtAttrs.hasOwnProperty(k)) {
        splitCtxtAttrs.push(splitCtxtHostDivs[i]["ctxt"][k]);
      } }
      splitDivs = recordHash(splitDivs, splitValsArray, "div", splitCtxtHostDivs[i]["div"]);
    } }
    
    // Iterate over all the observed combinations of split attribute key/value pairs, showing each sub-trace
    // within its requested div
    mapHash(splitTDL, function(splitCtxtAttrVals, rKey, dataList, indent) {
      var div = getHash(splitDivs, splitCtxtAttrVals, "div");
      if(div) displayTraceProjection(traceLabel, dataList, div, projectCtxtAttrs, traceAttrs, viz, loc); 
      else    displayTraceProjection(traceLabel, dataList, hostDivID, projectCtxtAttrs, traceAttrs, viz, loc); 
      });
  }
}*/

function displayTrace(traceLabel, hostDivID, ctxtAttrs, traceAttrs, viz, showFresh, showLabels) {
  var numContextAttrs=0;
  for(i in ctxtAttrs) { if(ctxtAttrs.hasOwnProperty(i)) { numContextAttrs++; } }
  
  var hostDiv = document.getElementById(hostDivID);
  
  // Create a version of traceList[traceLabel] where observations with different values for the context keys
  // in splitCtxtAttrs is placed in separate lists
  
  if(viz == 'table') {
    // NOTE: we always overwrite prior contents regardless of the value of showFresh, although this can be fixed in the future
    
    showTable(traceDataList[traceLabel], hostDivID, ctxtAttrs[0]);
    
    /*var ctxtCols = [];
    for(i in ctxtAttrs) { if(ctxtAttrs.hasOwnProperty(i)) {
      ctxtCols.push({key:ctxtAttrs[i], label:ctxtAttrs[i], sortable:true});
    } }
    
    var traceCols = [];
    for(i in traceAttrs) { if(traceAttrs.hasOwnProperty(i)) {
      traceCols.push({key:traceAttrs[i], label:traceAttrs[i], sortable:true});
    } }
    
    var columns = [];
    if(numContextAttrs>0) columns.push({label:"Context", children:ctxtCols});
    columns.push({label:"Trace",   children:traceCols});
    
    hostDiv.innerHTML += "<div class=\"example yui3-skin-sam\" id=\""+hostDivID+"-Table\"></div>";
    
    var tgtDiv = document.getElementById(hostDivID+"-Table");
    tgtDiv.style.width=1000;
    
    YUI().use("datatable-sort", function (Y) {
        // A table from data with keys that work fine as column names
        var traceTable = new Y.DataTable({
            columns: columns, 
            data   : traceDataList[traceLabel],
            caption: traceLabel
        });
        
        //traceTable.render("#div"+blockID);
        traceTable.render("#"+hostDivID+"-Table");
      });*/
  } else if(viz == 'lines') {
    if(numContextAttrs!=1) { alert("Line visualizations require requre exactly one context variable for each chart"); return; }
    
    var cStr=ctxtAttrs[0].replace(/:/g, "-");
    var newDiv="";
    if(showLabels) newDiv += ctxtAttrs[0] + "\n";
    newDiv += "<div id=\""+hostDivID+"_"+cStr+"\" style=\"height:300\"></div>\n";
    
    if(showFresh) hostDiv.innerHTML =  newDiv;
    else          hostDiv.innerHTML += newDiv;
    
    var data = [];
    for(i in traceDataList[traceLabel]) { if(traceDataList[traceLabel].hasOwnProperty(i)) {
    for(t in traceAttrs) { if(traceAttrs.hasOwnProperty(t)) {
      data.push([traceDataList[traceLabel][i][ctxtAttrs[0]], 
                 traceDataList[traceLabel][i][traceAttrs[t]]]);
    } } } }
    showScatterplot(data, hostDivID+"_"+cStr);
    
    /*
    // Compute the minimum and maximum value among all the trace attributes to be shown in this scatter 
    // plot to ensure that the y-axis is broad enough to include them all
    var minVal=1e100, maxVal=-1e100;
    for(i in traceAttrs) { if(traceAttrs.hasOwnProperty(i)) {
        if(minVal>minData[traceAttrs[i]]) 
          minVal=minData[traceAttrs[i]]; } }
    for(i in traceAttrs) { if(traceAttrs.hasOwnProperty(i)) {
        if(maxVal<maxData[traceAttrs[i]]) 
          maxVal=maxData[traceAttrs[i]]; } }
    
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
            categoryKey: ctxtAttrs[0],
            seriesKeys: traceAttrs,
            axes:myAxes,
            //render: "#div"+blockID+"_"+ctxtAttrs[0]
            render: "#"+hostDivID+"_"+ctxtAttrs[0]
        });
      });*/
  } else if(viz == 'decTree') {
    if(numContextAttrs==0) { alert("Decision Tree visualizations require one or more context variables"); return; }
    
    var newDiv="";
    if(showLabels) newDiv += traceAttrs[0] + "\n";
    newDiv += "<div id=\""+hostDivID+"_"+traceAttrs[0]+"\" style=\"height:300\"></div>\n";
    
    if(showFresh) hostDiv.innerHTML =  newDiv;
    else          hostDiv.innerHTML += newDiv;
    
    //if(!displayTraceCalled.hasOwnProperty(traceLabel)) {
    traceDataList[traceLabel] = _(traceDataList[traceLabel]);
    var model = id3(traceDataList[traceLabel], traceAttrs[0], ctxtAttrs);
    //alert(document.getElementById(hostDivID).innerHTML)
    // Create a div in which to place this attribute's decision tree
    //document.getElementById(hostDivID).innerHTML += traceAttrs[0]+"<div id='div"+blockID+":"+traceAttrs[0]+"'></div>";
    drawGraph(model,hostDivID+"_"+traceAttrs[0]);
  } else if(viz == 'boxplot') {
    var margin = {top: 10, right: 50, bottom: 20, left: 50},
        width = 120 - margin.left - margin.right,
        height = 500 - margin.top - margin.bottom;

    var divsForBoxplot = "";
    if(numContextAttrs>0) {
      for(c in ctxtAttrs) { if(ctxtAttrs.hasOwnProperty(c)) {
      for(t in traceAttrs) {   if(traceAttrs.hasOwnProperty(t)) {
        if(showLabels) divsForBoxplot += "Context=" + ctxtAttrs[c] + ", Trace=" + traceAttrs[t] + "\n";
        // Escape problematic characters
        var cStr=ctxtAttrs[c].replace(/:/g, "-");
        var tStr=traceAttrs[t].replace(/:/g, "-");
        divsForBoxplot += "<div id=\"" + hostDivID + "_" + cStr + "_" + tStr + "\"></div>\n";
      } } } }
    } else {
      for(t in traceAttrs) {   if(traceAttrs.hasOwnProperty(t)) {
        // Escape problematic characters
        var tStr=traceAttrs[t].replace(/:/g, "-");
        if(showLabels) divsForBoxplot += "Trace=" + traceAttrs[t] + "\n";
        divsForBoxplot += "<div id=\"" + hostDivID + "_" + tStr + "\"></div>\n";
      } }
    }
    
    if(document.getElementById(hostDivID)) {
      if(showFresh)
        document.getElementById(hostDivID).innerHTML = divsForBoxplot;
      else 
        /*if(loc == "showBegin")    */document.getElementById(hostDivID).innerHTML = divsForBoxplot + document.getElementById(hostDivID).innerHTML;
        //else if(loc == "showEnd") document.getElementById(hostDivID).innerHTML += divsForBoxplot;
      
      
      if(numContextAttrs>0) {
        for(c in ctxtAttrs) { if(ctxtAttrs.hasOwnProperty(c)) {
        for(t in traceAttrs) { if(traceAttrs.hasOwnProperty(t)) {
          //showBoxPlot(traceDataList[traceID], hostDivID+"_"+ctxtAttrs[0]+"_"+traceAttrs[0], ctxtAttrs[0], traceAttrs[0], width, height, margin);
          // Escape problematic characters
          var cStr=ctxtAttrs[c].replace(/:/g, "-");
          var tStr=traceAttrs[t].replace(/:/g, "-");
          showBoxPlot(traceDataList[traceLabel], hostDivID + "_" + cStr + "_" + tStr, ctxtAttrs[c], traceAttrs[t], width, height, margin);
        } } } }
      } else {
        for(t in traceAttrs) {   if(traceAttrs.hasOwnProperty(t)) {
          // Escape problematic characters
          var tStr=traceAttrs[t].replace(/:/g, "-");
          showBoxPlot(traceDataList[traceLabel], hostDivID+"_"+tStr, "", traceAttrs[t], width, height, margin);
        } }
      }
    }
  } else if(viz == 'heatmap') {
    if(numContextAttrs!=2) { alert("Heatmap visualizations require exactly two context variables"); return; }
    
    var newDiv = "<div id=\""+hostDivID+"-Heatmap\"></div>";
    if(showFresh) hostDiv.innerHTML =  newDiv;
    else          hostDiv.innerHTML += newDiv;
    
    /* // Array of keys of the context variables. Only the first two are used.
    var ctxtKeys = [];
    for(ctxtKey in allCtxtVals) { if(allCtxtVals.hasOwnProperty(ctxtKey)) {
      ctxtKeys.push(ctxtKey);
    } }*/
    
    // Array of all the values of the first context key, in sorted order
    var ctxt0KeyVals = [];
    for(ctxt0Key in allCtxtVals[ctxtKeys[traceLabel][0]]) { if(allCtxtVals[ctxtKeys[traceLabel][0]].hasOwnProperty(ctxt0Key)) { ctxt0KeyVals.push(ctxt0Key); } }
    ctxt0KeyVals.sort(getCompareFunc(ctxtValType[ctxtKeys[traceLabel][0]]));

    // Array of all the values of the second context key, in sorted order
    var ctxt1KeyVals = [];
    for(ctxt1Key in allCtxtVals[ctxtKeys[traceLabel][1]]) { if(allCtxtVals[ctxtKeys[traceLabel][1]].hasOwnProperty(ctxt1Key)) { ctxt1KeyVals.push(ctxt1Key); } }
    ctxt1KeyVals.sort(getCompareFunc(ctxtValType[ctxtKeys[traceLabel][1]]));
    
    // Create the gradient to be used to color the tiles
    var numColors = 1000;
    var colors = gradientFactory.generate({
        from: "#0000FF",
        to: "#FF0000",
        stops: numColors
    });
    // The values of attributes will be placed into numColors buckets, each marked with a different color.
    // This is the size of each bucket for each trace attribute
    var valBucketSize = [];
    
    // Prepare the data array that holds the info on the heatmaps to be shown (top-level of array, one 
    // sub-array per entry in traceAttrs) and the individual tiles in each heatmap (second-level array,
    // one entry for each pair of items in ctxt0KeyVals and ctxt1KeyVals)
    var data = [];
    for(traceAttrIdx in traceAttrs) { if(traceAttrs.hasOwnProperty(traceAttrIdx)) {
      valBucketSize[traceAttrIdx] = (maxData[traceAttrs[traceAttrIdx]] - minData[traceAttrs[traceAttrIdx]])/numColors;

      // attrData records the row and column of each tile in its heatmap (separate heatmap for each trace attribute), 
      // along with the index of the trace attribute in traceAttrs.
      var attrData = [];
      for(k1 in ctxt1KeyVals) { if(ctxt1KeyVals.hasOwnProperty(k1)) {
      for(k0 in ctxt0KeyVals) { if(ctxt0KeyVals.hasOwnProperty(k0)) {
        var contextVals = {};
        contextVals[ctxtKeys[traceLabel][0]] = ctxt0KeyVals[k0];
        contextVals[ctxtKeys[traceLabel][1]] = ctxt1KeyVals[k1];
        
        var traceVals = getDataHash(traceDataHash[traceLabel], contextVals);
        var traceLinks = getDataHash(traceLinkHash[traceLabel], contextVals);
        // If there is a record for this combination of context key values, add it to the dataset
        if(traceVals && traceLinks)
          attrData.push({row: k1, 
                         col:k0, 
                         traceAttrIdx:traceAttrIdx,
                         traceVals:traceVals, 
                         traceLinks:traceLinks});
      } } } }
      
      // Add the data for the current trace attribute to the dataset
      data.push(attrData);
    } }
    var tileWidth=20;
    var tileHeight=20;
    var titleHeight=20;
    var titleGap=5;
    
    var container = 
           d3.select("#"+hostDivID+"-Heatmap").selectAll("svg")
                  .data(data)
                .enter()
                .append("svg")
                  .attr("width",  tileWidth*ctxt0KeyVals.length)
                  .attr("height", titleHeight + titleGap + tileHeight*ctxt0KeyVals.length)
                  .attr("x", 0)
                  .attr("y", 1000);
    
    var title = container.append("text")
               .text(function(d, i) { /*alert(traceAttrs[i]);*/return traceAttrs[i]; })
               .attr("text-anchor", "middle")
               .attr("x", (tileWidth*ctxt0KeyVals.length)/2)
               .attr("y", titleHeight)
               .attr("fill", "#000000")
               .attr("font-family", "sans-serif")
               .attr("font-size", titleHeight+"px");
    //titleHeight = title.node().getBBox()["height"];
    
    container.selectAll("g")
                 .data(function(d, i) { return data[i]; })
      .enter()
      .append("g")
        .attr("width",  tileWidth)
        .attr("height", tileHeight)
        .attr("transform", function(d) { return "translate("+(d["col"]*tileWidth)+","+(d["row"]*tileHeight+titleHeight+titleGap)+")"; })
      .append("rect")
        .attr("width",  tileWidth)
        .attr("height", tileHeight)
        .attr("x", 0)
        .attr("y", 0)
        .attr("fill", function(d, i) {
          var valBucket = Math.floor((d["traceVals"][traceAttrs[d["traceAttrIdx"]]] - minData[traceAttrs[d["traceAttrIdx"]]]) / valBucketSize[d["traceAttrIdx"]]);
          return colors[Math.min(valBucket, colors.length-1)];
          })
        .on("click", function(d) {
          eval(d["traceLinks"][traceAttrs[d["traceAttrIdx"]]]);
          return true;
          });
         
  }
  
  displayTraceCalled = true;
}

