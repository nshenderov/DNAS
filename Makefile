MODULENAME = dnas
CXXFLAGS = -std=c++11 -pedantic -Werror -Wall -Wextra -O3 -fPIC $(INCLUDES)
INCLUDES = -I./framework/build/include $(patsubst %, -I%, $(shell find ./concrete/modules -type d))

DBGEXEOBJ = $(MODULENAME)_dbg.o
DBGEXE = ./export_dbg/$(MODULENAME)_dbg.out
DBGEXTERNAl = -L./framework/build/bin -Wl,-rpath='$$ORIGIN/bin' -lframework \
		   -L./concrete/export_dbg/bin -Wl,-rpath='$$ORIGIN/bin' -lconcrete_dbg

EXEOBJ = $(MODULENAME).o
EXE = ./export/$(MODULENAME).out
EXTERNAl = -L./framework/build/bin -Wl,-rpath='$$ORIGIN/bin' -lframework \
		   -L./concrete/export/bin -Wl,-rpath='$$ORIGIN/bin' -lconcrete

ex: CXXFLAGS += -DNDEBUG
ex: dirs build_framework concrete_rel $(EXE)
	cp -n ./framework/build/bin/libframework.so ./export/bin/libframework.so
	cp -n ./README.md ./export/README.md

ex_dbg: CXXFLAGS += -g
ex_dbg: dirs build_framework concrete_dbg $(DBGEXE)
	cp -n ./framework/build/bin/libframework.so ./export_dbg/bin/libframework.so

$(EXE): $(EXEOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(EXEOBJ) $(EXTERNAl)

$(DBGEXE): $(DBGEXEOBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(DBGEXEOBJ) $(DBGEXTERNAl)

%_dbg.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

dirs: 
	@mkdir -p ./export/bin ./export_dbg/bin

c: concrete_c framework_c
	rm -f ./export/*.out ./export/*.md ./export/*/*.so ./*.o ./concrete/modules/*/*/*.o
	rm -f ./export_dbg/*.out ./export_dbg/*/*.so

cl:
	rm -f ./bin/*/*.so

concrete_dbg:
	$(MAKE) -C ./concrete/ lib_dbg
	rsync -aP ./concrete/export_dbg/* ./export_dbg/

concrete_rel:
	$(MAKE) -C ./concrete/ lib_rel
	rsync -aP ./concrete/export/* ./export/

build_framework:
	$(MAKE) -C ./framework/ build

concrete_c:
	$(MAKE) -C ./concrete/ c

framework_c:
	$(MAKE) -C ./framework/ c cl c_hard c_build

.PHONY: c cl