function clickHierGraph(nodeid, buttonid, nameinod, mouX, mouY)
{
  //alert(mouX+"-"+mouY);
  //alert("node"+nodeid+"-"+buttonid + "-"+nameinod);
  var div = document.createElement('div');
  div.id = 'CanvizBox_node'+nodeid;
  div.style='position: absolute; top:'+mouY+'%; left:'+mouX+'%';
  //div.style='position:fixed; top:'+mouY+'%; left:'+mouX+'%';
  //div.innerHTML = "<a onclick=\"ClickOnHierGraphNode("+nodeid", this, "+buttonid+");\" target=\"_self\" href=\"#\">"+nameinod+"</a>";
  //div.innerHTML = '<a onclick="ClickOnHierGraphNode(node1, this, 3);" target="_self" href="#">'+nameinod+'</a>';
  document.getElementById('canvas').appendChild(div);
  ClickOnHierGraphNode('node'+nodeid, this, buttonid);
}
function addvizMeth(vizM)
{
    //alert("vizM = "+ vizM);
    var div = document.createElement('div');
    div.id = 'CanvizBox_node1';
    div.innerHTML = '<a onclick="ClickOnHierGraphNode(node1, this, 3);" target="_self" href="#">Neighbors</a>';
    document.getElementById('content').appendChild(div);
}

function dataset()
{
  data = getQueryParam("data");  
  attrNames = getQueryParam("attrNames"); 
  minVals = getQueryParam("minVals"); 
  maxVals = getQueryParam("maxVals"); 
  numContextAttrs = getQueryParam("numContextAttrs"); 
  numTraceAttrs = getQueryParam("numTraceAttrs"); 
  hostDivID = getQueryParam("hostDivID"); 
  nu = numTraceAttrs;
}

function getQueryParam(param) {
    var result =  window.location.search.match(
        new RegExp("(\\?|&)" + param + "(\\[\\])?=([^&]*)")
    );

    return result ? result[3] : false;
}

