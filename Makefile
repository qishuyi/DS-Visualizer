CXX	:= clang++
CXXFLAGS	:= -std=c++11 -g

all: visualizer list-test string-list-test binary-tree unbalanced-tree

clean:
	rm -rf visualizer
	rm -rf list-test
	rm -rf string-list-test
	rm -rf binary-tree
	rm -rf unbalanced-tree

visualizer: ptrace.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ -lrt -pthread

list-test: tests/list-test.cc
	$(CXX) -o $@ $^ -pthread

string-list-test: tests/string-list.cc
	$(CXX) -o $@ $^ -pthread

binary-tree: tests/binary_tree.cc
	$(CXX) -o $@ $^ -pthread

unbalanced-tree: tests/unbalanced_tree.cc
	$(CXX) -o $@ $^ -pthread
