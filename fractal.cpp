#include <chrono>
#include <complex>
#include <iostream>
#include <string>
#include <future>

#include <Magick++.h>

typedef std::complex<double> complex;

// Width and height of the output image
const int WIDTH = 1920 * 2;
const int HEIGHT = 1080 * 2;

// Number of threads to use when multithreading
const int NUM_THREADS = 4;

// Maximum number of iterations when computing the Julia fractal
const int MAX_ITER = 500;

std::tuple<int, int, int> palette(double x) {
    int r = x * 128;
    int g = x * 256;
    int b = x * 256;
    return std::make_tuple(r, g, b);
}

std::vector<unsigned char> julia_pixels(complex c, int width, int height,
                                        int row_b, int row_e) {
    std::vector<unsigned char> pix;
    for (int i = row_b; i < row_e; i++) {
        for (int j = 0; j < width; j++) {
            double x = -1.4 + 2.8 * j / width;
            double y = 1.4 - 2.8 * i / height;
            complex z(x, y);
            double smooth_color = 0;
            for (int iter = 0; iter < MAX_ITER; iter++) {
                double norm = std::norm(z);
                smooth_color += std::exp(-norm);
                if (norm > 2.0)
                    break;
                z = z * z + c;
            }
            smooth_color /= MAX_ITER;
            auto color = palette(smooth_color);
            pix.push_back(std::get<0>(color));
            pix.push_back(std::get<1>(color));
            pix.push_back(std::get<2>(color));
        }
    }
    return pix;
}

int main(int argc, char** argv) {
    using namespace Magick;
    using namespace std::chrono;

    InitializeMagick(*argv);

    // Julia Fractal function:
    // f(z) = z^2 - c
    complex c; //(-0.221, -0.713);
    std::cout << "Computing the Julia set for f(x) = x^2 - c; c = ";
    std::cin >> c;
    
    std::future<std::vector<unsigned char>> threads[NUM_THREADS];

    steady_clock::time_point start_time = steady_clock::now();

    for (int tid = 0; tid < NUM_THREADS; tid++) {
        int row_b = tid * HEIGHT / NUM_THREADS;
        int row_e = (tid + 1) * HEIGHT / NUM_THREADS;
        threads[tid] = std::async(std::launch::async, julia_pixels,
                                  c, WIDTH, HEIGHT, row_b, row_e);
    }

    std::vector<unsigned char> pix;
    for (int tid = 0; tid < NUM_THREADS; tid++) {
        threads[tid].wait();
        std::vector<unsigned char> partial = threads[tid].get();
        pix.insert(pix.end(), partial.begin(), partial.end());
    }
    
    Image image;
    image.read(WIDTH, HEIGHT, "RGB", CharPixel, &pix[0]);
    gaussianBlurImage blur(2, 0.4);
    blur(image);

    steady_clock::time_point end_time = steady_clock::now();
    long long time_millis = duration_cast<milliseconds>(end_time - start_time).count();
    std::cout << "Finished generating image, took " << time_millis << "ms." << std::endl;
    
    std::string c_pretty = (c.real() >= 0 ? "+" : "") + std::to_string(c.real()) +
                           (c.imag() >= 0 ? "+" : "") + std::to_string(c.imag()) + "j";
    std::string out_filename = "output/" + c_pretty + ".png";
    image.write(out_filename);
    system(("open " + out_filename).c_str());
    return 0;
}
