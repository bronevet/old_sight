function clickModule(nodeid, buttonid, nameinod, mouX, mouY)
{
  var div = document.createElement('div');
  div.id = 'CanvizBox_node'+nodeid;
  div.style='position: absolute; top:'+mouY+'%; left:'+mouX+'%';
  document.getElementById('canvas').appendChild(div);
  ClickOnModuleNode('node'+nodeid, this, buttonid);
}

function linkJS(link)
{
  eval(link);

  //alert("here - " + link);
  //var t = link;
  //var t = goToAnchor([], [1],  function() {loadSubFile(top.detail.document, [1], 'detail.0.body', 'div1-0-1_1_1_1_1', top.summary.document, 'summary.0.body', 'sumdiv1-0-1_1_1_1_1', 'script/script.0'); focusLinkDetail('1-0-1_1_1_1_1'); focusLinkSummary('1-0-1_1_1_1_1');});;
  //goToAnchor([], [1],  function() {loadSubFile(top.detail.document, [1], 'detail.0.body', 'div1-0-1_1_1_1_1', top.summary.document, 'summary.0.body', 'sumdiv1-0-1_1_1_1_1', 'script/script.0'); focusLinkDetail('1-0-1_1_1_1_1'); focusLinkSummary('1-0-1_1_1_1_1');});
  //goToAnchor([], [1],  function() {loadSubFile(top.detail.document, [1], 'detail.0.body', 'div1-0-1_1_1_1_1_2_1_1_1_1_1_1_1', top.summary.document, 'summary.0.body', 'sumdiv1-0-1_1_1_1_1_2_1_1_1_1_1_1_1', 'script/script.0'); focusLinkDetail('1-0-1_1_1_1_1_2_1_1_1_1_1_1_1'); focusLinkSummary('1-0-1_1_1_1_1_2_1_1_1_1_1_1_1');});
  //goToAnchor([], [1], function() {loadSubFile(top.detail.document, [1],  'detail.0.body', 'div1-0-1_1_1_1_1_2_1', top.summary.document, 'summary.0.body', 'sumdiv1-0-1_1_1_1_1_2_1', 'script/script.0'); focusLinkDetail('1-0-1_1_1_1_1_2_1'); focusLinkSummary('1-0-1_1_1_1_1_2_1');});
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

