# DS-Visualizer

A data structure visualizer



## Visualizer Using D3

The visualizer uses D3v3 to generate the graphical user interface. 


### Features

##### Time Selector

The visualizer allows the user to select a time that correspond to a 
sequential timeline of the program it logged. The gap represents a 
number of executions that `ptrace` skips over between the time steps 
where the data structure is recorded, but does not guarantee a
consistent relationship between the time step. 


##### Search

The visualizer has a search and a time selection function implemented. 
Search will highlight (by filling the circle of) any matching node 
within the visualization under the selected time, whose pointer address 
contain the search term the user entered into the search box. The 
search will persist across time change by the drop down selector, until 
the user hits the reset button. 


##### Mouseover

The visualizer supports mouseover on each node rendered. The tooltip 
that appears on hover display the information associated with each one 
of the nodes on the graph. The memory tracing side of the project had
not been able to fully inspect the structures each pointer value points 
to beyond other possible pointer values that reside in the struct. Thus 
that aspect of the project does not fully realize the functionality of
the tooltip. Though the implementation on the visualizer does support 
such and will display any of the child pointers within any memory it finds. 


### requires

From the root of the `visualizer.html`, `-.css`, `-.js`, the visualizer 
requires a file `output/nodedata.csv` and `output/treedata.csv`.

The files are white-space sensitive. 


##### nodedata.csv

nodedata.csv requires the header `node,type,data` prior to the payload 
for d3 and visualizer.js to successfully parse the file as a csv. This 
file is responsible for the data that display on mouseover each node, as
well as providing time line information for the drop down.

The file must also contain data from at least one time step in the file
immediately after the file header, in a new line, in the format `--,--,n`,
where n is a numerical value denoting a discrete time step. Within the 
time step lays the information for each node. Each variable associated 
with the node takes its own line in the format `ptr,_type,_data`, where

- `ptr`, is the pointer/node this payload is associated with,
- `_type`, is the type of the data,
- `_data`, is the data.

If multiple variables are associated with a single `ptr`, each must take 
up its own line. Each line must have exactly two commas, and spaces 
after a comma can be empty, but not before. 


##### treedata.csv 

nodedata.csv requires the header `name,parent` prior to the payload 
for d3 and visualizer.js to successfully parse the file as a csv. This 
file is responsible for the data that display the linkage between nodes.

The file must also contain data from at least one time step in the file
immediately after the file header, in a new line, in the format `--,n`,
where n is a numerical value denoting a discrete time step. Within the 
time step lays the information for the structure of the tree. Each link
between two nodes takes its own line in the format `ptr_child,ptr_parent`,
where,

- `ptr_child`, is the child node and,
- `ptr_parent`, is the parent node.

*Note: the line immeidately after a new/successive time step must be the
root of the tree for that time step, where the root falls under name, 
and null falls under parent, e.g. `0x0,null`, where 0x0 is the root of 
the tree.*


##### convert_pairs_dfs.py

This python script takes an input file `input.log` within the same 
root directory as the script itself and produces a valid
`treedata.csv` and `nodedata.csv` file for the visualizer. The script 
parses the input file with the following format: 

`time_step,ptr_parent,ptr_child`

where each line is a link between a parent and a child from the 
specified time step. This particular script has no support for 
generating additional data for nodedata except for child pointers. It is 
written to use in conjunction to our pointer-only memory tracer/
inspector and friendly to log written during depth-first traversals.

An example of the input is below:

```
0,0x0,0x00
0,0x00,0x000
0,0x000,0x0000
0,0x000,0x0001
0,0x00,0x001
0,0x001,0x0012
0,0x0,0x01
0,0x01,0x010
0,0x010,0x0102
0,0x01,0x011
0,0x011,0x0110
0,0x0110,0x01100
0,0x01100,0x011002
0,0x0110,0x01101
0,0x011,0x0111
0,0x0111,0x01112
1,0x0,0x00
1,0x00,0x000
1,0x000,0x0000
1,0x000,0x0001
1,0x00,0x001
1,0x001,0x0012
4,0x0,0x01
4,0x01,0x010
4,0x010,0x0102
4,0x01,0x011
4,0x011,0x0110
4,0x0110,0x01100
4,0x01100,0x011002
```

To run the script, run the terminal command `python convert_pairs_dfs.py`


##### convert_detailed_bfs.py

This python script takes an input file `input.log` within the same 
root directory as the script itself and produces a valid
`treedata.csv` and `nodedata.csv` file for the visualizer. The script 
parses the input file with the following format: 

```
# node time_slice _struct ptr_addr_0
# data _type _symbol _data
# data _type _symbol _data
# ptr _ptr ptr_addr1
# ptr _ptr ptr_addr2

# node time_slice _struct _ptr_addr_1
# data _type _symbol _data
# data _type _symbol _data
# ptr _ptr _ptr_addr3
# ptr _ptr _ptr_addr4
```

where each block is a c-styled struct declaration for any traced memory.  
This particular script supports struct specific data (payload) for 
nodedata, including child pointers. It is friendly to log written during
breadth-first traversals.

To run the script, run the terminal command `python convert_detailed_bfs.py`


## Acknowledgement

- tree inspired by http://bl.ocks.org/d3noob/fa0f16e271cb191ae85f
- mouseevents inspired by http://bl.ocks.org/WilliamQLiu/76ae20060e19bf42d774
- tooltip inspired by http://bl.ocks.org/d3noob/a22c42db65eb00d4e369 -->
- dropdown inspired by http://bl.ocks.org/williaster/10ef968ccfdc71c30ef8
- searchbox inspired by http://stackoverflow.com/questions/18167236 
