MODULENAME = minion
CXXAARCH = aarch64-linux-gnu-g++
CXXFLAGS = -std=c++11 -pedantic -Werror -Wall -Wextra -O3 $(INCLUDES)
INCLUDES = -I./include \
		   -I../../../framework/build/include \
		   -I../common
		   
DBGOBJS = $(patsubst %.cpp, %_dbg.o, $(wildcard ./*/*.cpp)) \
		  $(patsubst %.cpp, %_dbg.o, $(EXTERNSRCS))

RELOBJS = $(patsubst %.cpp, %.o, $(wildcard ./*/*.cpp)) \
		  $(patsubst %.cpp, %.o, $(EXTERNSRCS))

DBGOBJSAARCH = $(patsubst %.cpp, %_dbg_aarch64.o, $(wildcard ./*/*.cpp)) \
		       $(patsubst %.cpp, %_dbg_aarch64.o, $(EXTERNSRCS))

RELOBJSAARCH = $(patsubst %.cpp, %_aarch64.o, $(wildcard ./*/*.cpp)) \
		       $(patsubst %.cpp, %_aarch64.o, $(EXTERNSRCS))

EXTERNSRCS = $(FWMODULESPATH)/logger/src/logger.cpp \
		  	 $(FWMODULESPATH)/reactor/src/listener.cpp \
		  	 $(FWMODULESPATH)/reactor/src/reactor.cpp \
		  	 $(FWMODULESPATH)/thread_pool/src/thread_pool.cpp

HEADS = $(wildcard ./*/*.hpp)
DBGEXE = $(MODULENAME)_daemon_dbg.out
RELEXE = $(MODULENAME)_daemon.out
DBGEXEAARCH = $(MODULENAME)_daemon_aarch64_dbg.out
RELEXEAARCH = $(MODULENAME)_daemon_aarch64.out
FWMODULESPATH = ../../../framework/modules

ex_aarch: $(RELOBJSAARCH) $(RELEXEAARCH)
ex: $(RELOBJS) $(DBGEXE)

ex_aarch_dbg: CXXFLAGS += -g
ex_aarch_dbg: $(DBGOBJSAARCH) $(DBGEXEAARCH)

ex_dbg: CXXFLAGS += -g
ex_dbg: $(DBGOBJS) $(RELEXE)

%_aarch64.out: $(RELOBJSAARCH)
	$(CXXAARCH) $(CXXFLAGS) -o $@ $(RELOBJSAARCH)
	
%_aarch64.o: %.cpp $(HEADS)
	$(CXXAARCH) $(CXXFLAGS) -c -o $@ $<

%.out: $(RELOBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(RELOBJS)

%.o: %.cpp $(HEADS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<


%_dbg_aarch64.out: $(DBGOBJSAARCH)
	$(CXXAARCH) $(CXXFLAGS) -o $@ $(DBGOBJSAARCH)
	
%_dbg_aarch64.o: %.cpp $(HEADS)
	$(CXXAARCH) $(CXXFLAGS) -c -o $@ $<

%_dbg.out: $(DBGOBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(DBGOBJS)

%_dbg.o: %.cpp $(HEADS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

c:
	rm -f ./*.out ./*/*.o

.PHONY: c cl ex