MODULENAME = plugin_manager
INCLUDES = -I./include -I../dispatcher/include -I./test -I../utils
CXXFLAGS = -std=c++11 -pedantic -Werror -Wall -Wextra -O3 -fPIC -g -DPRINT_DEBUG $(INCLUDES)
SRCS = $(wildcard ./*/*.cpp)
DBGOBJS = $(patsubst %.cpp, %_dbg.o, $(filter-out ./*/test_plugin*.cpp, $(SRCS)))
HEADS = $(wildcard ./*/*.hpp)
EXE = $(MODULENAME)_test.out
PLUGINS_SRCS = $(wildcard ./*/test_plugin*.cpp)
PLUGINS_OBJS = $(PLUGINS_SRCS:.cpp=_dbg.o)
PLUGINS_LIBS = $(PLUGINS_SRCS:./test/%.cpp=./test/lib%.so)
PLUGINS_DIR = ./test/plugins

ex: mkdir $(DBGOBJS) $(PLUGINS_LIBS) $(EXE)

%.out: $(DBGOBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(DBGOBJS)

./test/lib%.so: ./test/%.o
	$(CC) $(CPPFLAGS) -shared -o $@ $<

%_dbg.o: %.cpp $(HEADS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

mkdir:
	@mkdir -p $(PLUGINS_DIR)

c:
	rm -f ./*.out ./*/*.o ./*/*.so

.PHONY: mkdir c cl ex