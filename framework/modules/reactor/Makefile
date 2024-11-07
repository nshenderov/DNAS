MODULENAME = reactor
INCLUDES = -I./include -I./test -I../utils -I../common
CXXFLAGS = -std=c++11 -pedantic -Werror -Wall -Wextra -O3 -g $(INCLUDES)
DBGOBJS = $(patsubst %.cpp, %_dbg.o, $(wildcard ./*/*.cpp))
HEADS = $(wildcard ./*/*.hpp)
EXE = $(MODULENAME)_test.out

ex: $(DBGOBJS) $(EXE)

%.out: $(DBGOBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(DBGOBJS)

%_dbg.o: %.cpp $(HEADS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

c:
	rm -f ./*.out ./*/*.o

.PHONY: c cl ex