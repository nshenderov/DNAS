MODULENAME = minion_proxy
INCLUDES = -I./include -I./test \
		   -I../../../framework/build/include \
		   -I../minion_manager/include \
		   -I../common \
		   -I../async_injection/include \
		   -I../async_injection/include \
		   -I../scheduler/include
CXXFLAGS = -std=c++11  -O3 -g $(INCLUDES)
DBGOBJS = $(patsubst %.cpp, %_dbg.o, $(wildcard ./*/*.cpp)) $(patsubst %.cpp, %_dbg.o, $(wildcard ../async_injection/src/*.cpp)) $(patsubst %.cpp, %_dbg.o, $(wildcard ../scheduler/src/*.cpp))
HEADS = $(wildcard ./*/*.hpp)
EXE = $(MODULENAME)_test.out

ex_dbg: $(DBGOBJS) $(EXE)

$(EXE): $(DBGOBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(DBGOBJS)

%_dbg.o: %.cpp $(HEADS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

c:
	rm -f ./*.out ./*/*.o

.PHONY: c cl ex