MODULENAME = logger
CXXFLAGS = -std=c++11 -pedantic -Werror -Wall -Wextra -O3 -g -fPIC $(INCLUDE)
HEADS = $(wildcard ./*/*.hpp)
EXE = $(MODULENAME)_test.out
DBGOBJS = $(patsubst %.cpp, %_dbg.o, $(filter-out ./test/logger_testing_plugin.cpp,$(wildcard ./*/*.cpp))) \
		  $(patsubst %.cpp, %_dbg.o, $(wildcard ../thread_pool/src/*.cpp)) 
INCLUDE = -I./include -I./src -I./test \
		  -I../pqueue/include \
		  -I../waitable_queue/include \
		  -I../thread_pool/include \
		  -I../handleton/include \
		  -I../utils

ex: $(DBGOBJS) $(EXE)

%.out: ./test/%_dbg.o ./test/liblogger_testing_plugin.so
	$(CXX) $(CXXFLAGS) -o $@ $< -L./test/ -Wl,-rpath=./test/ -llogger_testing_plugin

./test/liblogger_testing_plugin.so: ./test/logger_testing_plugin_dbg.o $(filter-out ./test/logger_test_dbg.o, $(DBGOBJS))
	$(CXX) $(CXXFLAGS) -shared -o $@ $< $(filter-out ./test/logger_test_dbg.o, $(DBGOBJS))

%_dbg.o: %.cpp $(HEADS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

c:
	rm -f ./*.out ./*/*.o ./*/*.so ./*.log

.PHONY: c cl ex