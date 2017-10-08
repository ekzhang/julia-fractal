CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=`Magick++-config --cppflags` -g -O2
CXXFLAGS=`Magick++-config --cxxflags` -std=c++11
LDFLAGS=`Magick++-config --ldflags` -g
LDLIBS=`Magick++-config --libs`

SRCS=src/fractal.cpp src/main.cpp
OBJS=$(patsubst src/%.cpp,build/%.o,$(SRCS))

all: build/main

run: all
	./build/main

build/main: $(OBJS)
	$(CXX) $(LDFLAGS) -o build/main $(OBJS) $(LDLIBS)

build/%.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c src/$*.cpp -o $@

build/main.o: src/main.cpp src/fractal.h

build/fractal.o: src/fractal.h src/fractal.cpp

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) build/main
