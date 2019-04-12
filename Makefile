CXX	:= clang
CXXFLAGS	:= --std=c++11 -g -Wall $(shell pkg-config --libs --cflags libelf++ libdwarf++)

all: visualizer

clean:
	rm -rf visualizer

visualizer: ptrace.cc
	$(CXX) $(CXXFLAGS) -o $@ $^
