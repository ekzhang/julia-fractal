CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=`Magick++-config --cppflags` -g -O2
CXXFLAGS=`Magick++-config --cxxflags` -std=c++11
LDFLAGS=`Magick++-config --ldflags` -g
LDLIBS=`Magick++-config --libs`

SRCS=src/fractal.cpp src/main.cpp src/animation.cpp
OBJS=$(patsubst src/%.cpp,build/%.o,$(SRCS))

all: build/main build/animation

run: all
	./build/main

build/main: build/main.o build/fractal.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

build/animation: build/animation.o build/fractal.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

build/%.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c src/$*.cpp

build/main.o: src/main.cpp src/fractal.h

build/animation.o: src/animation.cpp src/fractal.h

build/fractal.o: src/fractal.h src/fractal.cpp

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) build/main
