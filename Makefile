CXX	:= clang++
CXXFLAGS	:= -std=c++11

all: visualizer list-test string-list-test

clean:
	rm -rf visualizer
	rm -rf tests/list-test
	rm -rf tests/string-list-test

visualizer: ptrace.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ -lrt -pthread

list-test: tests/list-test.cc
	$(CXX) -o $@ $^ -pthread

string-list-test: tests/string-list.cc
	$(CXX) -o $@ $^ -pthread
