CXX	:= clang++
CXXFLAGS	:= -std=c++11

all: visualizer list-test

clean:
	rm -rf visualizer
	rm -rf list-test

visualizer: ptrace.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ -lrt -pthread

list-test: list-test.cc
	$(CXX) -o $@ $^ -pthread
