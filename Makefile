CXX	:= clang
CXXFLAGS	:=

all: visualizer

clean:
	rm -rf visualizer

visualizer: ptrace.c
	$(CXX) $(CXXFLAGS) -o $@ $^ -lrt
