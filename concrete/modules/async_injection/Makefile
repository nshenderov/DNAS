MODULENAME = async_injection
CXXFLAGS = -std=c++11 -pedantic -Werror -Wall -Wextra -O3 -g $(INCLUDE)
HEADS = $(wildcard ./*/*.hpp)
EXE = $(MODULENAME)_test.out
DBGOBJS = $(patsubst %.cpp, %_dbg.o, $(wildcard ./*/*.cpp)) \
		  $(patsubst %.cpp, %_dbg.o, $(wildcard ../scheduler/src/*.cpp)) 
INCLUDE = -I./include -I./src -I./test \
		  -I../scheduler/include \
		  -I../../../framework/build/include

ex: $(DBGOBJS) $(EXE)

%.out: $(DBGOBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(DBGOBJS)

%_dbg.o: %.cpp $(HEADS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

c:
	rm -f ./*.out ./*/*.o

.PHONY: c cl ex