#include <algorithm>
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

std::vector<double> julia_pixels(complex c, int width, int height,
                                 int row_b, int row_e) {
    std::vector<double> pix;
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
            pix.push_back(smooth_color);
        }
    }
    return pix;
}

void equalize(std::vector<double>& img) {
    std::vector<int> ordering(img.size());
    for (int i = 0; i < img.size(); i++) {
        ordering.push_back(i);
    }
    std::sort(ordering.begin(), ordering.end(), [&img](int i, int j) {
        return img[i] < img[j];
    });
    for (int i = 0; i < ordering.size(); i++) {
        img[ordering[i]] = 1.0 * i / ordering.size();
    }
}

int main(int argc, char** argv) {
    using namespace Magick;
    using namespace std::chrono;

    InitializeMagick(*argv);

    // Julia Fractal function:
    // f(z) = z^2 - c
    complex c; // Ex: (-0.221, -0.713);
    std::cout << "Computing the Julia set for f(x) = x^2 - c; c = ";
    std::cin >> c;

    steady_clock::time_point start_time = steady_clock::now();

    // Begin multithreaded Julia set calculations
    std::future<std::vector<double>> threads[NUM_THREADS];
    for (int tid = 0; tid < NUM_THREADS; tid++) {
        int row_b = tid * HEIGHT / NUM_THREADS;
        int row_e = (tid + 1) * HEIGHT / NUM_THREADS;
        threads[tid] = std::async(std::launch::async, julia_pixels,
                                  c, WIDTH, HEIGHT, row_b, row_e);
    }

    // Wait on the threads to finish, and combine results to form the image
    std::vector<double> raw_colors;
    for (int tid = 0; tid < NUM_THREADS; tid++) {
        threads[tid].wait();
        std::vector<double> partial = threads[tid].get();
        raw_colors.insert(raw_colors.end(), partial.begin(), partial.end());
    }

    // Normalization procedure
    equalize(raw_colors);

    // Apply the color palette
    std::vector<unsigned char> pix;
    for (double d : raw_colors) {
        auto t = palette(d);
        pix.push_back(std::get<0>(t));
        pix.push_back(std::get<1>(t));
        pix.push_back(std::get<2>(t));
    }
    
    Image image;
    image.read(WIDTH, HEIGHT, "RGB", CharPixel, &pix[0]);
    // gaussianBlurImage blur(2, 0.4);
    // blur(image);

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
