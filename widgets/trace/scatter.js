//var data = [[5,3], [10,17], [15,4], [2,8]];

// Adapted from http://bl.ocks.org/bunkat/2595950   

function createNumericScale(data, idx, minVisCoord, maxVisCoord) {
  var Min = d3.min(data, function(d) { return parseFloat(d[idx]); });
  var Avg = d3.sum(data, function(d) { return parseFloat(d[idx]); }) / data.length;
  var Max = d3.max(data, function(d) { return parseFloat(d[idx]); });

  // If there is a huge range in the x coordinates, use a log scale
  if(Min>0 && Max / Min > 1e3) 
    return d3.scale.log()
            .domain([Min, Max])
            .range([ minVisCoord, maxVisCoord ]);
  // Otherwise, use a linear scale
  else 
    return d3.scale.linear()
            .domain([0, Max])
            .range([ minVisCoord, maxVisCoord ]);  
}

function showScatterplot(data, hostDiv) {
  var margin = {top: 20, right: 15, bottom: 60, left: 60},
      width = document.getElementById(hostDiv).clientWidth - margin.left - margin.right,
      height = document.getElementById(hostDiv).clientHeight - margin.top - margin.bottom;
  
  // Determine whether the x and y axes are numeric or categorical
  var isXNumeric=true, isYNumeric=true;
  for(d in data) { if(data.hasOwnProperty(d)) {
    if(!isNumber(data[d][0])) { isXNumeric = false; }
    if(!isNumber(data[d][1])) { isYNumeric = false; }
    if(!isXNumeric && !isYNumeric) break;
  } }
  
  var x, y;
  
  if(isXNumeric) x = createNumericScale(data, 0, 0, width);
  else           x = d3.scale.ordinal()
                           .domain(data.map(function (d) {return d[0]; }))
                           .rangeRoundBands([0, width]);
  
  if(isYNumeric) y = createNumericScale(data, 1, height, 0);
  else           y = d3.scale.ordinal().range([height, 0]);
  
  var chart = d3.select("#"+hostDiv)
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
                 .scale(x)
                 .orient('bottom')
  
  main.append('g')
          .attr('transform', 'translate(0,' + height + ')')
          .attr('class', 'main axis date')
          .style({ 'stroke': 'black', 'fill': 'none', 'shape-rendering': 'crispEdges', 'font': '10px sans-serif'})
          .call(xAxis);
  
  // draw the y axis
  var yAxis = d3.svg.axis()
                  .scale(y)
                  .orient('left');
  
  main.append('g')
           .attr('transform', 'translate(0,0)')
           .attr('class', 'main axis date')
           .style({ 'stroke': 'black', 'fill': 'none', 'shape-rendering': 'crispEdges', 'font': '10px sans-serif'})
           .call(yAxis);
  
  var g = main.append("svg:g"); 
  
  g.selectAll("scatter-dots")
       .data(data)
       .enter().append("svg:circle")
           .attr("cx", function (d,i) { /*alert("d="+d+", i="+i+", x(d[0])="+x(d[0])+", y(d[1])="+y(d[1]));*/ 
                                         return (isXNumeric? x(parseFloat(d[0])): x(d[0]) + x.rangeBand()/2); } )
           .attr("cy", function (d) { return y(parseFloat(d[1])); } )
           .attr("r", 3)
           .style("fill", "red");
}
