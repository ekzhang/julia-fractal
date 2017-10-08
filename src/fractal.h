#pragma once
#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <functional>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include <Magick++.h>

typedef std::complex<double> complex;
typedef std::tuple<unsigned char, unsigned char, unsigned char> color;

// Width and height of the output image
const int DEFAULT_WIDTH = 3000;
const int DEFAULT_HEIGHT = 3000;

// Number of threads to use when multithreading
const int DEFAULT_NUM_THREADS = 4;

// Maximum number of iterations when computing the Julia fractal
const int DEFAULT_MAX_ITER = 256;

std::vector<double> julia_pixels(complex c, int width, int height,
                                 int max_iter, int row_b, int row_e);

void equalize(std::vector<double>& img);

void scale(std::vector<double>& img);

color palette(double x);

Magick::Image julia_set(const complex c,
                        const int width=DEFAULT_WIDTH,
                        const int height=DEFAULT_HEIGHT,
                        std::function<void(std::vector<double>&)> normalize=scale,
                        const int max_iter=DEFAULT_MAX_ITER,
                        const int num_threads=DEFAULT_NUM_THREADS);
