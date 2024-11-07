MODULENAME = dispatcher
INCLUDES = -I./include -I./test -I../utils
CXXFLAGS = -std=c++11 -pedantic -Werror -Wall -Wextra -O3 -g $(INCLUDES)
OBJS = $(patsubst %.cpp, %.o, $(wildcard ./*/*.cpp))
HEADS = $(wildcard ./*/*.hpp)
EXE = $(MODULENAME)_test.out

ex: $(OBJS) $(EXE)

%.out: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

$(OBJS): $(HEADS)

c:
	rm -f ./*.out ./*/*.o

.PHONY: ex c