MODULENAME = handleton
INCLUDES = -I./include -I./test -I../utils
CXXFLAGS = -std=c++11 -pedantic -Werror -Wall -Wextra -O3 -g -fPIC $(INCLUDES)
OBJS = $(patsubst %.cpp, %.o, $(wildcard ./*/*.cpp))
HEADS = $(wildcard ./*/*.hpp)
EXE = $(MODULENAME)_test.out
LDFLAGS = -L./test/ -Wl,-rpath=./test/
LIBS = $(patsubst ./test/%.o, ./test/lib%.so, $(filter-out ./test/%_test.o, $(OBJS)))

ex: $(OBJS) $(LIBS) $(EXE)

%.out: ./test/%.o ./test/lib$(MODULENAME)_module.so
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) -l$(MODULENAME)_module

./test/lib%.so: ./test/%.o
	$(CXX) $(CXXFLAGS) -shared -o $@ $<

$(OBJS): $(HEADS)

c:
	rm -f ./*.out ./*/*.o ./*/*.so

.PHONY: ex c