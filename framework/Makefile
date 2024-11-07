MODULENAME = framework
CXXFLAGS = -std=c++11 -pedantic -Werror -Wall -Wextra -O3 -fPIC $(INCLUDES)
INCLUDES = -I./include -I./src -I./test $(patsubst %, -I%, $(shell find ./modules -type d))
HEADS = $(wildcard ./include/*.hpp)
SRCSFILTER = ./modules/common/% ./modules/utils/%
SRCS =  $(filter-out $(SRCSFILTER), $(wildcard ./modules/*/src/*.cpp))
RELOBJS = $(patsubst %.cpp, %_rel.o, $(SRCS)) $(wildcard ./src/*.cpp)
DBGOBJS = $(patsubst %.cpp, %_dbg.o, $(SRCS)) $(wildcard ./src/*.cpp)
TESTOBJ = ./tests/$(MODULENAME)_test_dbg.o
TESTEXE = $(MODULENAME)_test.out
TESTPLUGINNAME = test_plugin
TESTPLUGINOBJ = ./tests/$(TESTPLUGINNAME)_dbg.o
TESTPLUGINLIB = ./tests/lib$(TESTPLUGINNAME)_dbg.so

ex: 
	@mkdir -p ./tests/plugins
ex: CXXFLAGS += -g -rdynamic
ex: $(DBGOBJS) $(TESTPLUGINLIB) $(TESTEXE)

lib_dbg: CXXFLAGS += -g
lib_dbg: $(DBGOBJS) ./bin/debug/lib$(MODULENAME)_dbg.so

lib_rel: CXXFLAGS += -DNDEBUG
lib_rel: $(RELOBJS) ./bin/release/lib$(MODULENAME).so

%.out: $(DBGOBJS) $(TESTOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(DBGOBJS) $(TESTOBJ)

./bin/release/lib$(MODULENAME).so: $(RELOBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(RELOBJS)

./bin/debug/lib$(MODULENAME)_dbg.so: $(DBGOBJS) 
	$(CXX) $(CXXFLAGS) -shared -o $@ $(DBGOBJS)

%_dbg.o: %.cpp $(HEADS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%_rel.o: %.cpp $(HEADS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

c:
	rm -f ./*.out ./*/*.o ./*/*.so ./*/*/*.so ./*.log

cl:
	rm -f ./bin/*/*.so

c_hard: c
	rm -f ./modules/*/*/*.o ./modules/*/*.out 

$(TESTPLUGINLIB): $(TESTPLUGINOBJ)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(TESTPLUGINOBJ)

build: lib_rel
	@mkdir -p ./build ./build/bin/ ./build/include/
	rsync -aP --exclude=dummy.md ./bin/release/* ./build/bin/
	find ./ -type f -name "*.hpp" \
	| xargs cp -nt ./build/include/

c_build:
	rm -r ./build

.PHONY: c cl c_hard ex lib_rel lib_dbg build c_build