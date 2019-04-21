
// ************** Generate the tree diagram	 *****************
var margin = {top: 20, right: 120, bottom: 20, left: 120},
	width = 960 - margin.right - margin.left,
	height = 500 - margin.top - margin.bottom;

var i = 0;

var tree = d3.layout.tree()
	.size([height, width]);

var diagonal = d3.svg.diagonal()
	.projection(function(d) { return [d.y, d.x]; });

var svg = d3.select("body").append("svg")
	.attr("width", width + margin.right + margin.left)
	.attr("height", height + margin.top + margin.bottom)
  .append("g")
	.attr("transform", "translate(" + margin.left + "," + margin.top + ")");


var div = d3.select("body").append("div")
    .attr("class", "tooltip")
    .style("opacity", 0);

var payload = [];
var root_global;
var data_byTime = [];

// load the external data
d3.csv("treedata.csv", function(error, data) {

  console.log(data);
  var time_current;
  data.forEach(function(link) {
    if (link.name == "--") {
      time_current = link.parent;
      data_byTime[time_current] = [];
    } else {
      console.log("DEBUG::: " + time_current + " " + link);
      data_byTime[time_current].push(link);
    }
  });

  // *********** Convert flat data into a nice tree ***************
  // create a name: node map
  var dataMap = data_byTime[1].reduce(function(map, node) {
    map[node.name] = node;
    return map;
  }, {});

  console.log("dataMap:", dataMap);

  // create the tree array
  var treeData = [];
  data_byTime[1].forEach(function(node) {
    // add to parent
    var parent = dataMap[node.parent];
    if (parent) {
      // create child array if it doesn't exist
      (parent.children || (parent.children = []))
        // add node to child array
        .push(node);
    } else {
      // parent is null or missing
      treeData.push(node);
    }
  });

  console.log(treeData);

  root = treeData[0];
  root_global = root;

  update(root, 1);
});


d3.csv("nodedata.csv", function(error,data) {

  var time;

  data.forEach(function(line) {
    //console.log("line" + line + " " + line.node + " " + line.type + " " + line.data);

    if (line.node == '--') {
      // advace time
      time = line.data;
      payload[time] = {};

      console.log(time);
    } else {

      var variable = {"type":line.type, "data":line.data};
      var p = payload[time];    // p is a time slice

      if (p[line.node] == null)
        p[line.node] = [];

      p[line.node].push(variable);
    }
  });

  render_dropdown(Object.keys(payload));
});


function update(source, time_shown) {

  console.log("Updating with time: "+ time_shown);

  // Clear canvas
  svg.selectAll("g.node").remove();
  svg.selectAll("circle").remove();

  // Compute the new tree layout.
  var nodes = tree.nodes(root).reverse(),
	  links = tree.links(nodes);

  // Normalize for fixed-depth.
  nodes.forEach(function(d) { d.y = d.depth * 180; });

  // Declare the nodesâ€¦
  var node = svg.selectAll("g.node")
	  .data(nodes, function(d) { return d.id || (d.id = ++i); });

  // Enter the nodes.
  var nodeEnter = node.enter().append("g")
	  .attr("class", "node")
	  .attr("transform", function(d) {
		  return "translate(" + d.y + "," + d.x + ")"; });

  nodeEnter.append("circle")
	  .attr("r", 10)
	  .style("fill", "#fff")
    // ADDED
    //.on("mouseover", handleMouseOver)
    //.on("mouseout", handleMouseOut);
    .on("mouseover", function(d) {
            div.transition()
                .duration(200)
                .style("opacity", .9);
            div.html(function() {
                        var p = payload[time_shown];
                        if (p[d.name] == null)
                          return "empty";
                        var ret = " ";
                        p[d.name].forEach(function(element) {
                          ret = ret + element.type + " = " + element.data + "<br> ";
                        });
                      return ret; })
                .style("left", (d3.event.pageX) + "px")
                .style("top", (d3.event.pageY - 28) + "px");
            })
        .on("mouseout", function(d) {
            div.transition()
                .duration(500)
                .style("opacity", 0);
        });

  nodeEnter.append("text")
	  .attr("x", function(d) {
		  return d.children || d._children ? -13 : 13; })
	  .attr("dy", ".35em")
	  .attr("text-anchor", function(d) {
		  return d.children || d._children ? "end" : "start"; })
	  .text(function(d) { return d.name; })
	  .style("fill-opacity", 1);

  // Declare the linksâ€¦
  var link = svg.selectAll("path.link")
	  .data(links, function(d) { return d.target.id; });

  // Enter the links.
  link.enter().insert("path", "g")
	  .attr("class", "link")
	  .attr("d", diagonal);

  node.exit().remove();
  link.exit().remove();

} // end update()


function render_dropdown (keys) {
  console.log("DEBUG:: Inside dropdown " + keys);

  node = document.getElementById("dropdown");
  keys.forEach(function(d) {
    console.log("<option onclick=\"update(root_global,"+d+")>"+d+"</option>");
    node.insertAdjacentHTML("beforeend","<option onclick=\"update(root_global,"+d+")\">"+d+"</option>");
  });

}
/*
var dropdownChange = function() {
    var time_new = d3.select(this).property('value');
    updateBars(root_global, time_new);
};

var dropdown = d3.select("#vis-container")
                    .insert("select", "svg")
                    .on("change", dropdownChange);

dropdown.selectAll("option")
    .data(function() {
      return Object.keys(payload); })
  .enter().append("option")
    .attr("value", function (d) { return d; })
    .text(function (d) { return d; });
*/
