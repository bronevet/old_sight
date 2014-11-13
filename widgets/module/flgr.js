function clickModule(nodeid, buttonid, nameinod, mouX, mouY)
{
  var div = document.createElement('div');
  div.id = 'CanvizBox_node'+nodeid;
  div.style='position: absolute; top:'+mouY+'%; left:'+mouX+'%';
  document.getElementById('canvas').appendChild(div);
  ClickOnModuleNode('node'+nodeid, this, buttonid);
}
function addvizMeth(vizM)
{
    //alert("vizM = "+ vizM);
    var div = document.createElement('div');
    div.id = 'CanvizBox_node1';
    div.innerHTML = '<a onclick="ClickOnModuleNode(node1, this, 3);" target="_self" href="#">Neighbors</a>';
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

function getGraName()
{
  graName = getQueryParam("graName");
}

function getQueryParam(param) {
    var result =  window.location.search.match(
        new RegExp("(\\?|&)" + param + "(\\[\\])?=([^&]*)")
    );

    return result ? result[3] : false;
}

