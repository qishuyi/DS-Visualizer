file = open("input.log", "r")
f_nodes = open("nodedata.csv", "w")
f_links = open("treedata.csv", "w")

# "Python foreach equivalent"
# "how to use dictionaries in Python"

# node time_slice _struct ptr_addr_0
# data _type _symbol _data
# data _type _symbol _data
# ptr _ptr ptr_addr1
# ptr _ptr ptr_addr2

# node time_slice _struct _ptr_addr_1
# data _type _symbol _data
# data _type _symbol _data
# ptr _ptr ptr_addr3
# ptr _ptr ptr_addr4

time_last = -1
f_line_number = 0

# write csv column headers
f_nodes.write("node,type,data\n")
f_links.write("name,parent\n")

# first layer are all time slices
epoch = {}
parents = {}
children = {}
root = {}

new_epoch = True

# read logfile
for line in file:

  f_line_number = f_line_number + 1

  words = line.rstrip().split(",")

  if not time_last == words[0]:
    new_epoch = True
  
  if len(words) != 3:
      raise Exception("Malformed input!\n  ...on line "+str(f_line_number)+": "+line)

  time_last = words[0]
  ptr_parent = words[1]
  ptr_child = words[2]

  # time slice !exist yet
  if not time_last in epoch.keys():
    # put line into time slice with line ptr_parent & ptr_child
    epoch[time_last] = {ptr_parent: [ptr_child]}

    # create new time in parents and children too
    parents[time_last] = [ptr_parent]
    children[time_last] = [ptr_child]

    root[time_last] = ptr_parent
    continue

  parents[time_last].append(ptr_parent)
  children[time_last].append(ptr_child)

  # time slice exists, but parent doesn't
  if not ptr_parent in epoch[time_last]:
    epoch[time_last][ptr_parent] = [ptr_child]
    continue

  # time slice exists and parent exists
  epoch[time_last][ptr_parent].append(ptr_child)



# end of file reading

print(parents)
print(children)

# now print stuff
for time in epoch.keys():

  # write epoch separator into file
  f_links.write("--," + time + "\n")
  f_nodes.write("--,--,"+ time + "\n")

  f_links.write(root[time] + ",null\n")

  # writes links to null for leaf nodes
  #for child in children[time]:
    
  #  if not child in parents[time]:
      
  #    f_links.write("null,"+child+"\n")
      

      
  for parent in epoch[time].keys():

    for child in epoch[time][parent]:
        
      #write to links
      f_links.write(child+","+parent+"\n")
      
      #write to nodes
      f_nodes.write(parent+",ptr,"+child+"\n")

  
  
    
# end of printing to file


    
file.close()
f_links.close()
f_nodes.close()
