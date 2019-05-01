CXX	:= clang++
CXXFLAGS	:= -std=c++11

all: visualizer

clean:
	rm -rf visualizer

visualizer: ptrace.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ -lrt -pthread
