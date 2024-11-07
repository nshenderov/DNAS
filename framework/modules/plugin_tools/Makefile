MODULENAME = plugin_tools
INCLUDES = -I./include -I./src -I../factory/include -I../handleton/include -I../common
CXXFLAGS = -std=c++11 -pedantic -Werror -Wall -Wextra -O3 -fPIC $(INCLUDES)
RELLIBOBJS = $(patsubst %.cpp, %_rel.o, $(wildcard ./src/*.cpp))
DBGLIBOBJS = $(patsubst %.cpp, %_dbg.o, $(wildcard ./src/*.cpp))
HEADS = $(wildcard ./*/*.hpp)

lib_rel: $(RELLIBOBJS) ../../bin/release/lib$(MODULENAME)_rel.a

lib_dbg: CPPFLAGS += -g
lib_dbg: $(DBGLIBOBJS) ../../bin/debug/lib$(MODULENAME)_dbg.a

# ../../bin/release/lib$(MODULENAME)_rel.so: $(RELLIBOBJS)
# 	$(CXX) $(CXXFLAGS) -shared -o $@ $(RELLIBOBJS)

# ../../bin/debug/lib$(MODULENAME)_dbg.so: $(DBGLIBOBJS)
# 	$(CXX) $(CXXFLAGS) -shared -o $@ $(DBGLIBOBJS)

../../bin/release/lib$(MODULENAME)_rel.a: $(RELLIBOBJS)
	ar rcs $@ $(RELLIBOBJS)

../../bin/debug/lib$(MODULENAME)_dbg.a: $(DBGLIBOBJS)
	ar rcs $@ $(DBGLIBOBJS)

%_dbg.o: %.cpp $(HEADS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%_rel.o: %.cpp $(HEADS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

c:
	rm -f ./*.out ./*/*.o

cl:
	rm -f ./bin/*/*.so

.PHONY: c cl ex lib_rel lib_dbg