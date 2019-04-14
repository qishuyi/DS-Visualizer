

## format requirement

### log file

Will most like takes on the form

```
node ptr_addr_0
_type _symbol _data
_type _symbol _data
_ptr ptr_addr1
_ptr ptr_addr2

node _ptr_addr_1
data _type _symbol:_data
data _type _symbol:_data
_ptr ptr_addr3
_ptr ptr_addr4
```


### d3 graph

Needs to be in separate node/link objects within a json

```
{
  "nodes": [
    {
      "id": ptr_addr0,
      "name": ptr_addr0
    },
    {
      "id": _ptr_addr1,
      "name": _ptr_addr1    
    },
    ...
  ],

  "links": [
    {
      "source": ptr_addr0,
      "target": ptr_addr1
    },
    {
      "source": ptr_addr0,
      "target": ptr_addr2    
    },
    ...
  ],
  "data": [
    {
      "id": ptr_addr0,
      [
        "_type _symbol": "_data",
        "_type _symbol": "_data",
        ...
      ]
    },
    {
      "id": ptr_addr1,
      [
        "_type _symbol": "_data",
        "_type _symbol": "_data",
        ...
      ]
    },
  ]
}

```

Additionally, we need a data object to carry extra payload, i.e. other data fields inside the struct tracked

### conversion process

Assume structs is a list of `element`s, and the `element` struct define as follows

```c
typedef struct element {
  type_t symbol_0;
  type_t symbol_1;
  element_t symbol_l;
  element_t symbol_r;
} element_t;

```

```
nodes = Array.new
links = Array.new
data = Array.new

elements.each do |s|
  nodes << s.ptr_addr
  links << [s.ptr_addr, s.ptr_addr_l]
  links << [s.ptr_addr, s.ptr_addr_r]
  data << [{"s.symbol+type":, data}, {"s.symbol+type":, data},...]  
end

open(file)

write("{}")
write(file, "\"nodes\":[\n")

nodes.each do |n|
  write("{\n\"id\":"+n+",\n\"name\"":+n+"\n},\n")
end

write("],\n\"data\":[")

links.each do |l|
  write("{\n\"source\":"+l[0]+",\n\"target\"":+l[1]+"\n},\n")
end

// write data

close (file)
```


## tooltip

link: https://www.d3-graph-gallery.com/graph/heatmap_tooltip.html


## timeline

### initial impression

use drop down to select specific time-slice, then use on-click time bar in chronology if time permits
