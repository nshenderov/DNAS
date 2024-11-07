MODULENAME = concrete
CXXFLAGS = -std=c++11 -pedantic -Werror -Wall -Wextra -O3 -fPIC $(INCLUDES)
INCLUDES = -I../framework/build/include $(patsubst %, -I%, $(shell find ./modules -type d))

HEADS = $(filter-out ./modules/_%, $(wildcard ./modules/*/*/*.hpp))
SRCS =  $(filter-out ./modules/_%, $(wildcard ./modules/*/src/*.cpp))
RELOBJS = $(patsubst %.cpp, %_rel.o, $(SRCS))
DBGOBJS = $(patsubst %.cpp, %_dbg.o, $(SRCS))

PLUGINS = $(wildcard ./plugins/*.cpp)
LIBSPLUGINSDBG = $(patsubst ./plugins/%.cpp, ./export_dbg/bin/lib%_plugin_dbg.so, $(PLUGINS))
LIBSPLUGINSREL = $(patsubst ./plugins/%.cpp, ./export/bin/lib%_plugin.so, $(PLUGINS))

lib_dbg: CXXFLAGS += -g
lib_dbg: dirs minion_daemon_dbg ./export_dbg/bin/lib$(MODULENAME)_dbg.so $(LIBSPLUGINSDBG)
# lib_dbg: dirs minion_daemon_arch_dbg ./export_dbg/bin/lib$(MODULENAME)_dbg.so $(LIBSPLUGINSDBG)

lib_rel: CXXFLAGS += -DNDEBUG
lib_rel: dirs minion_daemon_rel ./export/bin/lib$(MODULENAME).so $(LIBSPLUGINSREL)
# lib_rel: dirs minion_daemon_arch_rel ./export/bin/lib$(MODULENAME).so $(LIBSPLUGINSREL)

./export_dbg/bin/lib$(MODULENAME)_dbg.so: $(DBGOBJS) 
	$(CXX) $(CXXFLAGS) -shared -o $@ $(DBGOBJS)

./export/bin/lib$(MODULENAME).so: $(RELOBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(RELOBJS)

%_dbg.o: %.cpp $(HEADS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%_rel.o: %.cpp $(HEADS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

./export_dbg/bin/lib%_plugin_dbg.so: ./plugins/%_dbg.o
	$(CXX) $(CXXFLAGS) -shared -o $@ $<

./export/bin/lib%_plugin.so: ./plugins/%.o
	$(CXX) $(CXXFLAGS) -shared -o $@ $<

minion_daemon_dbg:
	$(MAKE) -C ./modules/_minion ex_dbg
	cp ./modules/_minion/*_dbg.out ./export_dbg/

minion_daemon_rel:
	$(MAKE) -C ./modules/_minion ex
	cp ./modules/_minion/*.out ./export/

minion_daemon_arch_dbg:
	$(MAKE) -C ./modules/_minion ex_aarch_dbg
	cp ./modules/_minion/*_dbg_aarch64.out ./export_dbg/

minion_daemon_arch_rel:
	$(MAKE) -C ./modules/_minion ex_aarch
	cp ./modules/_minion/*_aarch64.out ./export/

dirs: 
	@mkdir -p ./export/bin ./export_dbg/bin

c:
	rm -f ./*/*.out ./*/*/*.so ./*/*/*/*.o

cl:
	rm -f ./bin/*/*.so

.PHONY: c cl