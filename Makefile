run: build/fractal
	./build/fractal
build/fractal: fractal.cpp
	g++ -O2 -std=c++11 -o build/fractal fractal.cpp `Magick++-config --cppflags --cxxflags --ldflags --libs`