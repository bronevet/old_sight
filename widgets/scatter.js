//var data = [[5,3], [10,17], [15,4], [2,8]];

// Adapted from http://bl.ocks.org/bunkat/2595950   

function showScatterplot(data, hostDiv) {
  var margin = {top: 20, right: 15, bottom: 60, left: 60},
      width = document.getElementById(hostDiv).clientWidth - margin.left - margin.right,
      height = document.getElementById(hostDiv).clientHeight - margin.top - margin.bottom;
  
  var x = d3.scale.linear()
          .domain([0, d3.max(data, function(d) { return d[0]; })])
          .range([ 0, width ]);
  
  var y = d3.scale.linear()
        .domain([0, d3.max(data, function(d) { return d[1]; })])
        .range([ height, 0 ]);
  
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
           .attr("cx", function (d,i) { return x(d[0]); } )
           .attr("cy", function (d) { return y(d[1]); } )
           .attr("r", 8)
           .style("fill", "red");
}