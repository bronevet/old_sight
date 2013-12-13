// Hash maps divs to the data arrays that are shown in them
var div2Data = {};
function showTable(data, hostDiv, sortAttrName) {
  div2Data[hostDiv] = data;
  
  d3.select("#"+hostDiv).append("thead");
  d3.select("#"+hostDiv).append("tbody");
  
  refreshTable(hostDiv, sortAttrName);
}

function refreshTable(hostDiv, sortAttrName) {
  d3.select("#"+hostDiv).selectAll("tbody").selectAll("tr").remove();

// Header
    var th = d3.select("#"+hostDiv).selectAll("thead")
               .selectAll("th")
                     .data(jsonToArray(div2Data[hostDiv][0]))
                 .enter().append("th")
                    .attr("onclick", function (d, i) { return "refreshTable('"+ hostDiv+ "', '" + d[0] + "');";})
                    .text(function(d) { return d[0]; })

// Rows
    var tr = d3.select("#"+hostDiv).selectAll("tbody")
               .selectAll("tr")
                      .data(div2Data[hostDiv])
                 .enter().append("tr")
                    .sort(function (a, b) { return a == null || b == null ? 0 : stringCompare(a[sortAttrName], b[sortAttrName]); });

// Cells
    var td = tr.selectAll("td")
            .data(function(d) { return jsonToArray(d); })
          .enter().append("td")
            .attr("onclick", function (d, i) { return "transform('" + d[0] + "');";})
            .text(function(d) { return d[1]; });
}

function stringCompare(a, b) {
    a = a.toLowerCase();
    b = b.toLowerCase();
    return a > b ? 1 : a == b ? 0 : -1;
}

function jsonKeyValueToArray(k, v) {return [k, v];}

function jsonToArray(json) {
    var ret = new Array();
    var key;
    for (key in json) {
        if (json.hasOwnProperty(key)) {
            ret.push(jsonKeyValueToArray(key, json[key]));
        }
    }
    return ret;
};