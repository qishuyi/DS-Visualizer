file = open("input.log", "r")
f_nodes = open("nodedata.csv", "w")
f_links = open("treedata.csv", "w")

# node time_slice _struct _ptr_addr_0
# data _type _symbol _data
# data _type _symbol _data
# ptr _ptr _ptr_addr1
# ptr _ptr _ptr_addr2

# node time_slice _struct _ptr_addr_1
# data _type _symbol _data
# data _type _symbol _data
# ptr _ptr _ptr_addr3
# ptr _ptr _ptr_addr4

last_time = -1
last_struct = -1
last_ptr_addr = -1
first_in_time = True
f_line_number = 0

# write csv column headers
f_nodes.write("node,type,data\n")
f_links.write("name,parent\n")

# read logfile
for line in file:

  f_line_number = f_line_number + 1

  # inbetween data
  if line == "\n":
    continue

  line = line.rstrip()
  words = line.split(" ")
  # key = last_ptr_addr + ":" + last_time

  # load struct metadata
  # write timestamp to first nodedata
  if words[0] == "node" and len(words) == 4:
    
    # detect whether time has changed
    if last_time != words[1]:
      first_in_time = True
      
    last_time = words[1]
    last_struct = words[2]
    last_ptr_addr = words[3]
    
    if first_in_time: # on time-change, put head on links
      f_nodes.write("--,--,"+last_time+"\n")
      f_links.write("--,"+last_time+"\n")
      f_links.write(last_ptr_addr+",null\n")
      first_in_time = False
      
    continue
  
  # this is a variable
  #   put in dict_data
  if words[0] == "data" and len(words) == 4:
  #  data[key] = data[key].data.append({words[1]+" "+words[2]: words[3]})
    d_type = words[1]
    d_symbol = words[2]
    d_data = words[3]
    f_nodes.write(last_ptr_addr+","+d_type+" "+d_symbol+","+d_data+"\n")
    continue

  # this is a pointer..
  #   put into dict_data
  #   put into dict_links
  if words[0] == "ptr" and len(words) == 3:
  #  data[key] = data[key].data.append({words[1]+" "+words[2]: words[3]})
    d_symbol = words[1]
    d_data = words[2]
    f_nodes.write(last_ptr_addr+",ptr "+d_symbol+","+d_data+"\n")
    if d_data == "null" or d_data == "NULL":
      continue
    f_links.write(d_data+","+last_ptr_addr+"\n")
    continue

  raise Exception("Malformed input!\n  ...on line "+str(f_line_number)+": "+line)
    
file.close()
f_links.close()
f_nodes.close()
