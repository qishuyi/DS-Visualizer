
// ************** Generate the tree diagram	 *****************
var margin = {top: 20, right: 120, bottom: 20, left: 120},
	width = 2000 - margin.right - margin.left,
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

// create the tree array
var treeData = [];
var payload = [];
var root_global;
var data_byTime = [];
var time_default = 0;
var time_last = time_default;

var searching = false;
var search_for = "";

// load the external data
d3.csv("output/treedata.csv", function(error, data) {

  console.log("DEBUG::: treedata.csv, " + data);
  var time_current;
  var is_new = true;
  data.forEach(function(link) {
    if (link.name == "--") {
      time_current = link.parent;
      if (is_new) { time_default = time_current; is_new = false; }
      data_byTime[time_current] = [];
    } else {
      console.log("DEBUG::: " + time_current + " " + link);
      data_byTime[time_current].push(link);
    }
  });

  // *********** Convert flat data into a nice tree ***************
  // create a name: node map
  var dataMap = [];
  
  data_byTime.forEach(function(data_timeSlice, i) {
      dataMap[i] = data_timeSlice.reduce(function(map, node) {
                                      map[node.name] = node;
                                      return map; }, {});
  });
  console.log("DEBUG::: dataMap:", dataMap);

  
  data_byTime.forEach(function(data_timeSlice, i) {
    treeData[i] = [];
    data_timeSlice.forEach(function(node) {
      // add to parent
      var parent = dataMap[i][node.parent];
      if (parent) {
        // create child array if it doesn't exist
        (parent.children || (parent.children = []))
          // add node to child array
          .push(node);
      } else {
        // parent is null or missing
        treeData[i].push(node);
      }
    });
  });
  console.log("DEBUG::: tree data, " + treeData);

  root = treeData[time_default][0];
  root_global = root;

  update(root, time_default);
});


d3.csv("output/nodedata.csv", function(error,data) {
  var time;

  // Each line rerepsents a variable associated a node, under a timestamp
  // e.g. --,--,1           where 1 is a timestamp
  //      unique_address,type-symbol,data
  //
  data.forEach(function(line) {
    if (line.node == '--') {
      // Initialize object to store data associated with each time slice
      // TODO: Add checking code to warn non-sequential input
      time = line.data;
      payload[time] = {};

    } else {
      // Push variable into time slice, where line.node acts as a reference/hash
      var p = payload[time];    // Remove a layer of abstraction for p, a time slice
      
      if (p[line.node] == null) 
        p[line.node] = [];
      p[line.node].push({"type":line.type, "data":line.data});
    }
  });

  render_dropdown(Object.keys(payload));
});


function update(source, time_shown) {
  root = treeData[time_shown][0];
  time_last = time_shown;

  console.log("DEBUG::: Updating visualization with time: "+ time_shown);

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
	  .style("fill", function(d) {
      console.log(d);
      if (searching && d.name.includes(search_for))
        return "blue";
      else
        return "#fff";
      })
    .on("mouseover", function(d) {
            div.transition()
                .duration(200)
                .style("opacity", .9);
            div.html(print_struct_content(d,i,time_shown))
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

function search_dependent_fill (d) {
  console.log (d);
  return "#fff";
}


function render_dropdown (keys) {
  console.log("DEBUG::: Inside dropdown " + keys);
  node = document.getElementById("dropdown");
  
  keys.forEach(function(d) {
    console.log("DEBUG::: <option onclick=\"update(root_global,"+d+")>"+d+"</option>");
    node.insertAdjacentHTML("beforeend","<option onclick=\"update(root_global,"+d+")\">"+d+"</option>");
  });

}

function print_struct_content (d, i, time_shown) {
  console.log("DEBUG::: Inside print_struct_content ");
  var p = payload[time_shown];
  
  if (p[d.name] == null)
    return "empty";
    
  var ret = " ";
  p[d.name].forEach(function(element) {
    ret = ret + element.type + " = " + element.data + "<br> ";
  });
  
  return ret; 
}

function set_search() {
  search_for = document.getElementById("link_id").value;
  searching = true;
  console.log("DEBUG::: Searching for \"" + search_for + "\"");
  update(root_global, time_last);
}

function reset_search () {
  document.getElementById("link_id").value = "";
  searching = false;
  console.log("DEBUG::: Resetting tree");
  update(root_global, time_last);
}



