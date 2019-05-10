CXX	:= clang++
CXXFLAGS	:= -std=c++11

all: visualizer list-test

clean:
	rm -rf visualizer
	rm -rf list-test
	rm -rf list-test-long

visualizer: ptrace.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ -lrt -pthread

list-test: list-test.cc
	$(CXX) -o $@ $^ -pthread

list-test-long: list-test-long.cc
	$(CXX) -o $@ $^ -pthread
