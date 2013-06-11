	function unhide(divID) {
	  var parentDiv = document.getElementById("div"+divID);
		if (parentDiv) {
			// Hide the parent div
			parentDiv.className=(parentDiv.className=='hidden')?'unhidden':'hidden';
			// Get all the tables
			var childTbls = document.getElementsByTagName("table");
			condition = new RegExp("table"+divID+"[0-9_-]*");
			for (var i=0; i<childTbls.length; i++){ 
				var child = childTbls[i];
				// Set the visibility status of each child table to be the same as its parent div
				if ("table"+divID!=child.id && child.nodeType==1 && child.id!=undefined && child.id.match(condition)) {
				    child.className=parentDiv.className;
				}
			}
		}
	}

  function FileHash() {
    this.items = new HashTable();
    this.leaf  = new HashTable();
    this.objectType = "FileHash";
  }
  
  var fileInfo = new FileHash();
  
  function recordFile(fileID, rKey, rVal) {
      if(fileID.length==0) return;
      console.debug("recordFile("+fileID+", #"+fileID.length+", rKey="+rKey+", rVal="+rVal+"): "+fileInfoStr());
      fileInfo = recordFile_rec(fileInfo, fileID, rKey, rVal);
      console.debug("recordFile="+fileInfoStr());
  }
  
  function recordFile_rec(subfileInfo, fileID, rKey, rVal) {
    console.debug("recordFile_rec("+fileID+", #"+fileID.length+", rKey="+rKey+", rVal="+rVal+"), typeof subfileInfo="+(typeof subfileInfo)+": "+fileInfoStr());
    if(typeof subfileInfo === 'undefined') subfileInfo = new FileHash();
    
    var fileKey = fileID[0];
    console.debug("fileKey="+fileKey);
    if(fileID.length==1) {
      // If the given fileKey has not yet been mapped within this fileInfo sub-tree, create a HashTable for it
      if(!subfileInfo.leaf.hasItem(fileKey)) subfileInfo.leaf.setItem(fileKey, new HashTable());
      subfileInfo.leaf.getItem(fileKey).setItem(rKey, rVal);
    } else {
      // Pull off the first element from fileID
      fileID.splice(0, 1);
      subfileInfo.items.setItem(fileKey, recordFile_rec(subfileInfo.items.getItem(fileKey), fileID, rKey, rVal));
    }
    return subfileInfo;
  }
  
  function getFile(fileID, rKey) {
    console.debug("getFile: fileID="+fileID+", rKey="+rKey);
    console.debug(fileInfoStr());
      if(fileID.length==0) return;
      return getFile_rec(fileInfo, fileID, rKey);
  }
  
  function getFile_rec(subfileInfo, fileID, rKey) {
    var fileKey = fileID[0];
    if(fileID.length==1) {
      return subfileInfo.leaf.getItem(fileKey).getItem(rKey);
    } else {
      // Pull off the first element from fileID
      fileID.splice(0, 1);
      return getFile_rec(subfileInfo.items.getItem(fileKey), fileID, rKey);
    }
  }
  
  function fileInfoStr() {
    return fileInfoStr_rec(fileInfo, "");
  }
  
  function fileInfoStr_rec(subfileInfo, indent) {
    var str = "{\n";
    subfileInfo.leaf.each(function(k, v) {
      str += indent + "    L: " + k + " => \n";
      str += indent + "        "+v.str(indent + "        ")+"\n";
    });
    subfileInfo.items.each(function(k, v) {
      str += indent + "    I: " + k + " => ";
      if(v.objectType == "FileHash")
        str += fileInfoStr_rec(v, indent+"    ");
      else
        str += v;
      str += "\n";
    });
    return str + "\n" + indent + "}";
  }

	function loadURLIntoDiv(doc, url, divName, continuationFunc) {
		var xhr= new XMLHttpRequest();
		xhr.open('GET', url, true);
		xhr.onreadystatechange= function() {
			//Wait until the data is fully loaded
			if (this.readyState!==4) return;
			doc.getElementById(divName).innerHTML= this.responseText;
			if(typeof continuationFunc !== 'undefined')
			  continuationFunc();
		};
		xhr.send();
	}
	
  // From http://www.javascriptkit.com/javatutors/loadjavascriptcss.shtml
  //  and http://stackoverflow.com/questions/950087/how-to-include-a-javascript-file-in-another-javascript-file
  function loadjscssfile(filename, filetype, continuationFunc){
    if (filetype=="js"){ //if filename is a external JavaScript file
      var fileref=document.createElement('script')
      fileref.setAttribute("type","text/javascript")
      fileref.setAttribute("src", filename)
    }
    else if (filetype=="css"){ //if filename is an external CSS file
      var fileref=document.createElement("link")
      fileref.setAttribute("rel", "stylesheet")
      fileref.setAttribute("type", "text/css")
      fileref.setAttribute("href", filename)
    }
    
    if(typeof continuationFunc !== 'undefined') {
      fileref.onreadystatechange = continuationFunc;
      fileref.onload = continuationFunc;
    }
    
    if (typeof fileref!="undefined")
      document.getElementsByTagName("head")[0].appendChild(fileref)
  }
  
  function loadSubFile(detailDoc, detailURL, detailDivName, sumDoc, sumURL, sumDivName, scriptURL, continuationFunc) {
    loadURLIntoDiv(detailDoc, detailURL, detailDivName,
                   function() { 
                     loadURLIntoDiv(sumDoc, sumURL, sumDivName,
                                    function() { 
                                      loadjscssfile(scriptURL, "js", continuationFunc);
                                    } ); } );
    
    
  }

	function highlightLink(divID, newcolor) {
		var sumLink = top.summary.document.getElementById("link"+divID);
		sumLink.style.backgroundColor= newcolor;
	}
	function focusLink(divID, e) {
		e = e || window.event;
		if('cancelBubble' in e) {
			e.cancelBubble = true;
			top.summary.location = "summary.0.html#anchor"+divID;
		}
	}
	