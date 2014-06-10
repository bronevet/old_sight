function unhide(blockID) {
  var parentDiv = document.getElementById("div"+blockID);
  if (parentDiv) {
    // Hide the parent div
    parentDiv.className=(parentDiv.className=='hidden')?'unhidden':'hidden';
    // Get all the tables
    var childTbls = document.getElementsByTagName("table");
    condition = new RegExp("table"+blockID+"[0-9_-]*");
    for (var i=0; i<childTbls.length; i++){ 
      var child = childTbls[i];
      // Set the visibility status of each child table to be the same as its parent div
      if ("table"+blockID!=child.id && child.nodeType==1 && child.id!=undefined && child.id.match(condition)) {
          child.className=parentDiv.className;
      }
    }
  }
}

// Toggle's the visibility of a div and the table it contains.
// Unlike unhide, this function fully eliminates the div, including its title.
function toggleVisibility(blockID, visible) {
  var parentDiv = document.getElementById("div"+blockID);
  //alert("unhide("+blockID+") parentDiv="+parentDiv);
  if (parentDiv) {
    // Hide the parent div
    parentDiv.className=(visible?'unhidden':'hidden');
    // Get all the tables
    var childTbls = document.getElementsByTagName("table");
    condition = new RegExp("table"+blockID+"[0-9_-]*");
    for (var i=0; i<childTbls.length; i++){ 
      var child = childTbls[i];
      // Set the visibility status of each child table to be the same as its parent div
      //console.log("child.id="+child.id+" blockID="+blockID+" eq="+("table"+blockID==child.id));
      if ("table"+blockID==child.id)
        child.className=parentDiv.className;
    }
  }
}

function HTNode() {
  this.items = new HashTable();
  this.leaf  = new HashTable();
  this.objectType = "HTNode";
}

var fileInfo = new HTNode();

function recordFile(fileID, rKey, rVal) {
  fileInfo = recordHash(fileInfo, fileID, rKey, rVal);
/*    if(fileID.length==0) return;
    //console.debug("recordFile("+fileID+", #"+fileID.length+", rKey="+rKey+", rVal="+rVal+"): "+fileInfoStr());
    fileInfo = recordFile_rec(fileInfo, fileID, 0, rKey, rVal);
    console.debug("recordFile="+fileInfoStr());*/
}

/*function recordFile_rec(subfileInfo, fileID, index, rKey, rVal) {
  //console.debug("recordFile_rec("+fileID+", #"+fileID.length+", rKey="+rKey+", rVal="+rVal+"), typeof subfileInfo="+(typeof subfileInfo)+": "+fileInfoStr());
  if(typeof subfileInfo === 'undefined') subfileInfo = new HTNode();
  
  var fileKey = fileID[index];
  //console.debug("fileKey="+fileKey);
  if(index == fileID.length-1) {
    // If the given fileKey has not yet been mapped within this fileInfo sub-tree, create a HashTable for it
    if(!subfileInfo.leaf.hasItem(fileKey)) subfileInfo.leaf.setItem(fileKey, new HashTable());
    subfileInfo.leaf.getItem(fileKey).setItem(rKey, rVal);
  } else {
    subfileInfo.items.setItem(fileKey, recordFile_rec(subfileInfo.items.getItem(fileKey), fileID, index+1, rKey, rVal));
  }
  return subfileInfo;
}*/

function getFile(fileID, rKey) {
  console.debug("getFile: fileID="+fileID+", rKey="+rKey);
  console.debug(fileInfoStr());
  return getHash(fileInfo, fileID, rKey);
/*  return 
    if(fileID.length==0) return undefined;
    return getFile_rec(fileInfo, fileID, 0, rKey);*/
}

/*function getFile_rec(subfileInfo, fileID, index, rKey) {
  var fileKey = fileID[index];
  if(index == fileID.length-1) {
    return subfileInfo.leaf.getItem(fileKey).getItem(rKey);
  } else {
    return getFile_rec(subfileInfo.items.getItem(fileKey), fileID, index+1, rKey);
  }
}*/

function fileInfoStr() {
  return hash2Str(fileInfo);
  //return fileInfoStr_rec(fileInfo, "");
}

/*function fileInfoStr_rec(subfileInfo, indent) {
  var str = "{\n";
  subfileInfo.leaf.each(function(k, v) {
    str += indent + "    L: " + k + " => \n";
    str += indent + "        "+v.str(indent + "        ")+"\n";
  });
  subfileInfo.items.each(function(k, v) {
    str += indent + "    I: " + k + " => ";
    if(v.objectType == "HTNode")
      str += fileInfoStr_rec(v, indent+"    ");
    else
      str += v;
    str += "\n";
  });
  return str + "\n" + indent + "}";
}*/

// Given a hashtable, record the given key=>value mapping under the given UID, which is a list of keys
function recordHash(HT, UID, rKey, rVal) {
    if(UID.length==0) return undefined;
    return recordHash_rec(HT, UID, 0, rKey, rVal);
}

function recordHash_rec(subHT, UID, index, rKey, rVal) {
  //console.debug("recordFile_rec("+UID+", #"+UID.length+", rKey="+rKey+", rVal="+rVal+"), typeof subfileInfo="+(typeof subfileInfo)+": "+fileInfoStr());
  if(typeof subHT === 'undefined') subHT = new HTNode();
  
  var subKey = UID[index];
  //console.debug("subKey="+subKey);
  if(index == UID.length-1) {
    // If the given subKey has not yet been mapped within this fileInfo sub-tree, create a HashTable for it
    if(!subHT.leaf.hasItem(subKey)) subHT.leaf.setItem(subKey, new HashTable());
    subHT.leaf.getItem(subKey).setItem(rKey, rVal);
  } else {
    subHT.items.setItem(subKey, recordHash_rec(subHT.items.getItem(subKey), UID, index+1, rKey, rVal));
  }
  return subHT;
}

// Given a hashtable, push the given value to the list associated with given key, under the given UID, which is a list of keys
function pushHash(HT, UID, rKey, rVal) {
    if(UID.length==0) return undefined;
    return pushHash_rec(HT, UID, 0, rKey, rVal);
}

function pushHash_rec(subHT, UID, index, rKey, rVal) {
  //console.debug("recordFile_rec("+UID+", #"+UID.length+", rKey="+rKey+", rVal="+rVal+"), typeof subfileInfo="+(typeof subfileInfo)+": "+fileInfoStr());
  if(typeof subHT === 'undefined') subHT = new HTNode();
  
  var subKey = UID[index];
  //console.debug("subKey="+subKey);
  if(index == UID.length-1) {
    // If the given subKey has not yet been mapped within this fileInfo sub-tree, create a HashTable for it
    if(!subHT.leaf.hasItem(subKey)) {
      subHT.leaf.setItem(subKey, new HashTable());
      subHT.leaf.getItem(subKey).setItem(rKey, [rVal]);
    } else {
      var curList = subHT.leaf.getItem(subKey);
      curList.push(rVal);
    }
  } else {
    subHT.items.setItem(subKey, pushHash_rec(subHT.items.getItem(subKey), UID, index+1, rKey, rVal));
  }
  return subHT;
}

// Given a hashtable, return the value mapped to the given combination of key and UID, which is a list of keys
function getHash(HT, UID, rKey) {
  if(UID.length==0) return undefined;
  return getHash_rec(HT, UID, 0, rKey);
}

function getHash_rec(subHT, UID, index, rKey) {
  var subKey = UID[index];
  if(index == UID.length-1) {
    return subHT.leaf.getItem(subKey).getItem(rKey);
  } else {
    return getHash_rec(subHT.items.getItem(subKey), UID, index+1, rKey);
  }
}

// Returns a string representation of the given hashtable
function hash2Str(HT) {
  return hash2Str_rec(HT, "");
}

function hash2Str_rec(subHT, indent) {
  var str = "{\n";
  subHT.leaf.each(function(k, v) {
    str += indent + "    L: " + k + " => \n";
    v.each(function(k2, v2) {
      if(v2.objectType == "Array") {
        str += indent + "        [";
        for(i in v2) { if(v2.hasOwnProperty(i)) {
          str += v2[i]+" ";
        } }
        str += "]\n";
      } else
        str += indent + "        "+v2+"\n";
      });
  });
  subHT.items.each(function(k, v) {
    str += indent + "    I: " + k + " => ";
    if(v.objectType == "HTNode")
      str += hash2Str_rec(v, indent+"    ");
    else
      str += v;
    str += "\n";
  });
  return str + "\n" + indent + "}";
}


// Maps the given function to each key list / key=>value combination stored in the hashtable,
// providing as arguments mapFunc(UID, key, val, indent), where indent makes it easy to print human-readable debug output
function mapHash(HT, mapFunc) {
  mapHash_rec(HT, [], mapFunc, "");
}

function mapHash_rec(subHT, UID, mapFunc, indent) {
  subHT.leaf.each(function(k, v) {
    mapFunc(UID, k, v, indent);
  });
  subHT.items.each(function(k, v) {
    if(v.objectType == "HTNode") {
      var subUID = UID; subUID.push(k);
      mapHash_rec(v, subUID, mapFunc, indent+"    ");
    }
  });
}


/***************************
 ***** File Management *****
 ***************************/

var scriptEltID=0;
function loadURLIntoDiv(doc, url, divName, continuationFunc) {
  var xhr= new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.onreadystatechange= function() {
    //Wait until the data is fully loaded
    if (this.readyState!==4) return;
    // Option 1:
    //doc.getElementById(divName).innerHTML= this.responseText;
    // Option 2:
    var scriptNode = document.createElement('script_'+scriptEltID);
    scriptEltID++;
    scriptNode.innerHTML = this.responseText;
    doc.getElementById(divName).appendChild(scriptNode);
          
    if(typeof continuationFunc !== 'undefined')
      continuationFunc();
  };
  xhr.send();
}
  
// From http://www.javascriptkit.com/javatutors/loadjavascriptcss.shtml
//  and http://stackoverflow.com/questions/950087/how-to-include-a-javascript-file-in-another-javascript-file
function loadjscssfile(filename, filetype, continuationFunc){
  if (filetype=="text/css"){ //if filename is an external CSS file
    var fileref=document.createElement("link")
    fileref.setAttribute("rel", "stylesheet")
    fileref.setAttribute("type", "text/css")
    fileref.setAttribute("href", filename)
  //if filename is a external script file
  } else if(filetype=="text/javascript") { 
    var fileref=document.createElement('script')
    fileref.setAttribute("type", filetype)
    fileref.setAttribute("src", filename)
  } else {
    alert("ERROR: unknown file type \""+filetype+"\" for file name \""+filename+"\"!");
    return;
  }
  
  if(typeof continuationFunc !== 'undefined') {
    fileref.onreadystatechange = continuationFunc;
    fileref.onload = continuationFunc;
  }
  
  if (typeof fileref!="undefined")
    document.getElementsByTagName("head")[0].appendChild(fileref)
}

// Loads the given file and calls continuationFunc() on its contents
function loadFile(url, continuationFunc) {
  var xhr= new XMLHttpRequest();
  xhr.open('GET', url, true);
  xhr.onreadystatechange= function() {
    //Wait until the data is fully loaded
    if (this.readyState!==4) return;
    continuationFunc(this.responseText);
  };
  xhr.send();
}

// Loads the given file. The file is assumed to contain the paths of scripts, one per line.
// After the loading is finished, calls continuationFunc()
function loadScriptsInFile(doc, url, continuationFunc) {
  loadFile(url, function(text) {
      var lines=text.split("\n");
      
      function recursiveLoad(lines, i) {
        if(i<lines.length && lines[i] != "") {
          var fields = lines[i].split(" ");
          loadjscssfile(fields[0], fields[1],
            function() {
              recursiveLoad(lines, i+1);
            }
          )
        } else 
          continuationFunc();
      }
      recursiveLoad(lines, 0);
    });
}

function loadSubFile(detailDoc, fileID, detailURL, detailDivName, sumDoc, sumURL, sumDivName, scriptURL, continuationFunc) {
  if(!getFile(fileID, "loaded")) {
    loadURLIntoDiv(detailDoc, detailURL, detailDivName,
                   function() { 
                     loadURLIntoDiv(sumDoc, sumURL, sumDivName,
                                    function() { 
                                      loadjscssfile(scriptURL+".prolog", "text/javascript", 
                                        function() {loadjscssfile(scriptURL, "text/javascript", 
                                          function() { loadjscssfile(scriptURL+".epilog", "text/javascript", continuationFunc); } 
                                        ); }
                                      );
                                    } ); } );
  }
}

function loadAnchorScriptsFile(anchorFileID, continuationFunc) {
  if(!(anchorFileID in loadedAnchors))
    return loadjscssfile('script/anchor_script.'+anchorFileID, 'text/javascript', 
              function() { loadedAnchors[anchorFileID]=1; return continuationFunc(); } );
  else
    return continuationFunc();
}

function highlightLink(blockID, newcolor) {
  var sumLink = top.summary.document.getElementById("link"+blockID);
  if(sumLink) sumLink.style.backgroundColor=newcolor;
}
function focusLinkSummary(blockID, e) {
  if(typeof e !== 'undefined') {
    e = e || window.event;
    if('cancelBubble' in e) {
      e.cancelBubble = true;
      top.summary.location = "summary.0.html#anchor"+blockID;
    }
  } else {
    top.summary.location = "summary.0.html#anchor"+blockID;
  }
}

function focusLinkDetail(blockID) {
	top.detail.location = "detail.0.html#anchor"+blockID;
}


// Anchors
var anchors = new HashTable();
var loadedAnchors = {};

function anchor(fileID, blockID) {
  this.fileID  = fileID;
  this.blockID = blockID;
  console.debug("anchor("+fileID+", "+blockID+")");
}

// Opens the given file ID (if needed) and shifts the browser's focus to it.
// The file ID is encoded in the pair prefix/suffix. Initially prefix is empty and suffix contains the entire
// fileID. In each iteration we shift one element from suffix to prefix, open the file at that ID and repeat.
// The function modifies the arguments, so they cannot be used again after this call.
// When the anchor opens all the required files, it called continuationFunc, if it is provided
function goToAnchor(prefix, suffix, continuationFunc) {
  //console.debug("goToAnchor(["+prefix+"], ["+suffix+"])");
  // If the suffix is empty, we're done
  if(suffix.length == 0) {
    if(typeof continuationFunc !== 'undefined')
      return continuationFunc();
    else 
      return undefined;
  }
  
  // Move an entry from suffix to prefix
  prefix.push(suffix.splice(0, 1)[0]);
  
  // If this fileID has not yet been loaded
  if(!getFile(prefix, "loaded")) {
    console.debug('Loading '+prefix); 
    getFile(prefix, 'loadFunc')(
      function() { goToAnchor(prefix, suffix, continuationFunc); }
    );
  // Otherwise, if it has already been loaded, load its child file within the file ID
  } else {
    goToAnchor(prefix, suffix, continuationFunc);
  }
  return undefined;
}

var hostname="";
/*  function findReachableHost(hosts) {
  for(var i=0; i<hosts.length; i++)
  {
    if(hostReachable(hosts[i])) {
      hostname = hosts[i];
      return;
    }
  }
}

function hostReachable(hostname) {
  // Handle IE and more capable browsers
  var xhr = new ( window.ActiveXObject || XMLHttpRequest )( "Microsoft.XMLHTTP" );
  var status;
  
  // Open new request as a HEAD to the root hostname with a random param to bust the cache
  xhr.open( "HEAD", hostname, false );
  
  // Issue request and handle response
  try {
    xhr.send();
    return ( xhr.status >= 200 && xhr.status < 300 || xhr.status === 304 );
  } catch (error) {
    return false;
  }
}*/

function hostnameReachable(name) {
//alert("hostnameReachable("+name+")");
  hostname = name;
}

function getHostLink(url) {
  return "http://"+hostname+url;
}

function setGDBLink(link, url) {
  link.href = "http://"+hostname+url;
  //alert("Setting link to to "+link.href);
  return true;
}

/*  var next = function() { 
  var next = function() { 
    var next = undefined;
    console.debug('Loaded [1, 2, 2, 1]='+getFile([1, 2, 2, 1], 'loaded')); 
    if(!getFile([1, 2, 2, 1], 'loaded')) { 
      console.debug('Loading [1, 2, 2, 1]'); 
      getFile([1, 2, 2, 1], 'loadFunc')(next); } 
    else if(typeof next !== 'undefined') 
    { next(); }
    undefined;
  };
  console.debug('Loaded [1, 2, 2]='+getFile([1, 2, 2], 'loaded')); 
  if(!getFile([1, 2, 2], 'loaded')) { 
    console.debug('Loading [1, 2, 2]'); 
    getFile([1, 2, 2], 'loadFunc')(next);
  } else if(typeof next !== 'undefined') { next(); } 
  undefined;
};
console.debug('Loaded [1, 2]='+getFile([1, 2], 'loaded')); 
if(!getFile([1, 2], 'loaded')) { 
  console.debug('Loading [1, 2]'); 
  getFile([1, 2], 'loadFunc')(next);
} 
else if(typeof next !== 'undefined') { next(); } 
undefined;*/

