/***********************************
 ***** The Attributes Database *****
 ***********************************/
var attributes;

// Begin public utility /getVarType/
// Returns 'Function', 'Object', 'Array',
// 'String', 'Number', 'Boolean', or 'Undefined',
function getVarType( data ) {
  if (undefined === data ){ return 'Undefined'; }
  if (data === null ){ return 'Null'; }
  return {}.toString.call(data).slice(8, -1);
};  
// End public utility /getVarType/

function taffy2Str(taffy_map) {
  var msg_text = "";
  for ( key_name in taffy_map ){
    if ( taffy_map.hasOwnProperty( key_name ) ) {
      data_val = taffy_map[ key_name ];
      var_type = getVarType( data_val );

      switch ( var_type ){
        case 'Object'   :
          if ( data_val.hasOwnProperty( 'get' ) ){
            msg_text += JSON.stringify( data_val.get() );
          }
          else {
            msg_text += JSON.stringify( data_val );
          }
          break;
        case 'Number' :
          msg_text += String( data_val );
          break;

        default :
          msg_text += JSON.stringify( data_val );
      }

      msg_text += '\n\n';
    }
  }
  return msg_text;
}

var attrKey2AllVals = {};
function recordAttr(record, divID) {
  //console.log("record="+JSON.stringify(record));
  
  if(attributes === undefined)
    attributes = TAFFY( [] );
  
  for(keyName in record) { if(record.hasOwnProperty(keyName)) {
    // Update attrKey2AllVals to map each key to all the values the key is mapped to in any attribute
    //console.log(keyName+", exists="+(keyName in attrKey2AllVals));
    if(!(keyName in attrKey2AllVals))
      attrKey2AllVals[keyName] = {};
    (attrKey2AllVals[keyName])[record[keyName]] = true;
  } }
  
  record.divID = divID;
  attributes.insert(record);
  
  //console.log(taffy2Str(attributes().filter({ key_i : 3}).get()));
  //alert("attrKey2AllVals="+JSON.stringify(attrKey2AllVals));
}

// Encodes all the queries that need to be pertformed as a map from key names to arrays of their acceptable values
var allQueries;

function writeKeyValTable(id, viewType) {
  resetKeyValTable(id);
  
  var tgtDiv = document.getElementById(id);
  var str = "";
  
  str += "<table border=1 bgcolor=\"#ABA7FC\">\n";
  
  var numKeys=0;
  
  // Header
  str += "  <tr>";
  for(keyName in attrKey2AllVals) { if(attrKey2AllVals.hasOwnProperty(keyName)) {
    str += "<td bgcolor=\"#FCAB03\" style=\"font-size:large; font-weight:bold; text-align:center;\">"+keyName.substring(4)+"</td>";
    numKeys++;
  } }
  str += "</tr>\n";
  
  // Create an alternate representation of attrKey2AllVals that maps each key to 
  // an array of its corresponding values, rather than a hash.
  var attrKey2AllValsArray = {};
  var maxValsPerKey=-1; // The maximum number of values any key is mapped to
  for(keyName in attrKey2AllVals) { if(attrKey2AllVals.hasOwnProperty(keyName)) {
    attrKey2AllValsArray[keyName] = Object.keys(attrKey2AllVals[keyName]);
    if(maxValsPerKey < attrKey2AllValsArray[keyName].length) maxValsPerKey = attrKey2AllValsArray[keyName].length;
  } }
  
  // Body with the values mapped to each key
  for(var i=0; i<maxValsPerKey; i++) {
    str += "  <tr>";
    for(keyName in attrKey2AllValsArray) { if(attrKey2AllVals.hasOwnProperty(keyName)) {
      str += "<td style=\"text-align:center;\">";
      if(i<attrKey2AllValsArray[keyName].length) {
        var value = attrKey2AllValsArray[keyName][i];
        str += "<input type=\"checkbox\" name=\""+keyName+"\" value=\""+value+"\"";
        str += " onclick='attrCheckClick(this);'>\n";
        str += value;
      }
      str += "</td>";
    } }
    str += "</tr>\n";
  }
  
  str += "<tr><td colspan=\""+numKeys+"\" style=\"text-align:center;\"><a href=\"javascript:filterByAttr('"+viewType+"')\">";
  if(viewType == "add")         str += "<img src=\"img/attrAdd.gif\"    width=50 height=46 alt=\"Add by Attribute\">";
  else if(viewType == "remove") str += "<img src=\"img/attrRemove.gif\" width=50 height=46 alt=\"Remove by Attribute\">";
  else if(viewType == "only")   str += "<img src=\"img/attrOnly.gif\"   width=50 height=46 alt=\"Filter Only Attribute\">";
  str += "</a></td></tr>";
  
  str += "</table>\n";
  
  // Initialize allQueries. When the attribute request is submitted we issue the 
  // queries it encodes to the attributes database.
  allQueries = {};
  
  //alert(str);
  tgtDiv.innerHTML = str;
  
  AddDivPlacementEvents(function () { PlaceFixedDiv(id, true); });
}

function resetKeyValTable(id) {
  var tgtDiv = document.getElementById(id);
  tgtDiv.innerHTML = ""
}

// Adds the information from this checkbox's click to the query
function attrCheckClick(checkbox) {
  if(!(checkbox.name in allQueries))
    allQueries[checkbox.name] = new Array();
  allQueries[checkbox.name].push(checkbox.value);
}

// Performs the query and views or hides the selected blocks
function filterByAttr(viewType) {
  // The query object that we'll incrementally build up in filterByAttr_subQuery
  query = {};
  filterByAttr_subQuery(Object.keys(allQueries), 0, query, viewType);
  
  
  resetKeyValTable('attrTable');
}

function filterByAttr_subQuery(keys, keyIdx, query, viewType) {
  if(keyIdx < keys.length) {
    if(allQueries.hasOwnProperty(keys[keyIdx])) {
      for(var i in allQueries[keys[keyIdx]]) { //if(allQueries[keys[keyIdx]].hasOwnProperty(value)) {
        if(viewType == "add" || viewType == "remove") {
          query[keys[keyIdx]] = {'==':allQueries[keys[keyIdx]][i]};
          filterByAttr_subQuery(keys, keyIdx+1, query, viewType);
        } else if(viewType == "only") {
          query[keys[keyIdx]] = {'==':allQueries[keys[keyIdx]][i]};
          filterByAttr_subQuery(keys, keyIdx+1, query, "add");
          
          query[keys[keyIdx]] = {'!=':allQueries[keys[keyIdx]][i]};
          filterByAttr_subQuery(keys, keyIdx+1, query, "remove");
        }
        delete query[keys[keyIdx]];
      } //}
    }
  // If we've constructed the current query
  } else {
    //alert("filterByAttr("+viewType+")");
    attributes(query).each(function (match) {
      //alert("match="+JSON.stringify(match));
      toggleVisibility(match.divID, (viewType=='add'? true: false));
    });
  }
}