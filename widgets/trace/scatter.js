//var data = [[5,3], [10,17], [15,4], [2,8]];

// Adapted from http://bl.ocks.org/bunkat/2595950   

function createNumericScale_2DScatter(data, idx, minVisCoord, maxVisCoord, axisType) {
  var Min = d3.min(data, function(d) { return parseFloat(d[idx]); });
  var Avg = d3.sum(data, function(d) { return parseFloat(d[idx]); }) / data.length;
  var Max = d3.max(data, function(d) { return parseFloat(d[idx]); });
  
  // If this axis is compatible with the log visualization and
  // If it is selected to be log or it is not specified and there is a huge range in the x coordinates, use a log scale
  if(Min>0 && (axisType=="log" || (axisType===undefined && Max / Min > 1e2)))
    return ["log", 
            d3.scale.log()
                .domain([Min, Max])
                .range([ minVisCoord, maxVisCoord ])];
  // Otherwise, use a linear scale
  else {
    var domainMin, domainMax;
    // If the domain has only one element, widen it out to enable d3 to create a proper scale
    if(Min==Max) {
      domainMin = Min-1;
      domainMax = Min+1;
    } else {
      // If the region between Min and Max is far away from 0 or overlaps 0, show exactly this region
      if((Min<0 && Max>0) || (Min>0 && (Max-Min) < Max/2)) {
        domainMin = Min;
        domainMax = Max;
      // Otherwise, start the plotting region from 0
      } else {
        domainMin = 0;
        domainMax = Max;
      }
    }
  
    return ["lin",
            d3.scale.linear()
                .domain([domainMin, domainMax])
                .range([ minVisCoord, maxVisCoord ])];
  }
}

// Caches the data arrays of different hostDivs to make it possible to re-visualize the contents of
// different scatterplots interactively.
var cachedData = {};

function showScatterplot(data, hostDivID, xAxisType, yAxisType) {
  // Empty out the hostDiv
  document.getElementById(hostDivID).innerHTML="";
  
  // If data is provided, cache it; If it is not provided (call from an on-click handler, grab it from the cache)
  if(data === undefined) data = cachedData[hostDivID];
  else                   cachedData[hostDivID] = data;
  
  var margin = {top: 20, right: 15, bottom: 60, left: 60},
      width = document.getElementById(hostDivID).clientWidth - margin.left - margin.right,
      height = document.getElementById(hostDivID).clientHeight - margin.top - margin.bottom;
  
  // Determine whether the x and y axes are numeric or categorical
  var isXNumeric=true, isYNumeric=true;
  for(d in data) { if(data.hasOwnProperty(d)) {
    if(!isNumber(data[d][0])) { isXNumeric = false; }
    if(!isNumber(data[d][1])) { isYNumeric = false; }
    if(!isXNumeric && !isYNumeric) break;
  } }
  
  // Create the gradient to be used to color the tiles
  /*var colors = gradientFactory.generate({
      from: "#0000FF",
      to: "#FF0000",
      stops: data.length
  });*/
  
  var x, y;
  
  if(isXNumeric && xAxisType != "cat") 
    x = createNumericScale_2DScatter(data, 0, 0, width, xAxisType);
  else
    x = ["cat", d3.scale.ordinal()
                    .domain(data.map(function (d) {return d[0]; }))
                    .rangeRoundBands([0, width])];
  xAxisType = x[0];
  
  if(isYNumeric && yAxisType != "cat")
    y = createNumericScale_2DScatter(data, 1, height, 0, yAxisType);
  else
    y = ["cat", d3.scale.ordinal().range([height, 0])];
  yAxisType = y[0];
  
  var chart = d3.select("#"+hostDivID)
                  .append('svg:svg')
                  .attr('width', width + margin.right + margin.left)
                  .attr('height', height + margin.top + margin.bottom)
                  .attr('class', 'chart')
  
  var main = chart.append('g')
               .attr('transform', 'translate(' + margin.left + ',' + margin.top + ')')
               .attr('width', width)
               .attr('height', height)
               .attr('class', 'main')   
    
  // draw the x axis
  var xAxis = d3.svg.axis()
                 .scale(x[1])
                 .orient('bottom')
  
  main.append('g')
          .attr('transform', 'translate(0,' + height + ')')
          .attr('class', 'main axis date')
          .style({ 'stroke': 'black', 'fill': 'none', 'shape-rendering': 'crispEdges', 'font': '10px sans-serif'})
          .attr("onclick", "showScatterplot(undefined, '"+hostDivID+"', '"+(xAxisType=="lin"? "log": (xAxisType=="log"? "lin": "cat"))+"', '"+yAxisType+"');")
          .call(xAxis);
  
  // draw the y axis
  var yAxis = d3.svg.axis()
                  .scale(y[1])
                  .orient('left');
  
  main.append('g')
           .attr('transform', 'translate(0,0)')
           .attr('class', 'main axis date')
           .style({ 'stroke': 'black', 'fill': 'none', 'shape-rendering': 'crispEdges', 'font': '10px sans-serif'})
           .attr("onclick", "showScatterplot(undefined, '"+hostDivID+"', '"+xAxisType+"', '"+(yAxisType=="lin"? "log": (yAxisType=="log"? "lin": "cat"))+"');")
           .call(yAxis);
  
  var g = main.append("svg:g"); 
  
  g.selectAll("scatter-dots")
       .data(data)
       .enter().append("svg:circle")
           .attr("cx", function (d,i) { //console.log("d="+d+", i="+i+", x("+d[0]+")="+x[1](parseFloat(d[0]))+", y("+d[1]+")="+y[1](parseFloat(d[1]))+", xAxisType="+xAxisType);
                                         return (xAxisType=="log" || xAxisType=="lin" ? 
                                                    x[1](parseFloat(d[0])): 
                                                    x[1](parseFloat(d[0])) + x[1].rangeBand()/2); } )
           .attr("cy", function (d) { return y[1](parseFloat(d[1])); } )
           .attr("r", 3)
           .style("fill", "red"/*function(d,i) { return colors[i]; }*/ );
}
