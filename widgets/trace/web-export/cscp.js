/*
var dt = [];
var atNames = [];
var minVa = [], maxVa = [];
var numCt = [], numTra = 8;
var hosID = [];

function showCirSCP(data, attrNames, minVals, maxVals, numCtxtVars, numTraceAttrs, hostDivID)
{
    numTra = numTraceAttrs;
    document.write(numTraceAttrs);
}
*/

function dataset()
{
  //nu = location.search.substring(1);
  //nu = getQueryParam("nTra");
  data = getQueryParam("data");  
  attrNames = getQueryParam("attrNames"); 
  minVals = getQueryParam("minVals"); 
  maxVals = getQueryParam("maxVals"); 
  numContextAttrs = getQueryParam("numContextAttrs"); 
  numTraceAttrs = getQueryParam("numTraceAttrs"); 
  hostDivID = getQueryParam("hostDivID");
  
  showFresh = getQueryParam("showFresh");
  showLabels = getQueryParam("showLabels");
  refreshView = getQueryParam("refreshView");
  ctxtStr = getQueryParam("ctxtStr");
  traceStr = getQueryParam("traceStr");
     
  nu = numTraceAttrs;
}

function getQueryParam(param) {
    var result =  window.location.search.match(
        new RegExp("(\\?|&)" + param + "(\\[\\])?=([^&]*)")
    );

    return result ? result[3] : false;
}

