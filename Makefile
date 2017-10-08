fractal: build/fractal
	./build/fractal
build/fractal: src/fractal.cpp
	g++ -O2 -std=c++11 -o build/fractal src/fractal.cpp `Magick++-config --cppflags --cxxflags --ldflags --libs`
animation: build/animation
	./build/animation
build/animation: src/animation.cpp src/fractal.cpp
	g++ -O2 -std=c++11 -o build/animation src/animation.cpp `Magick++-config --cppflags --cxxflags --ldflags --libs`
