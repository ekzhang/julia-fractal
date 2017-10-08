#include <algorithm>
#include <chrono>
#include <complex>
#include <iostream>
#include <string>
#include <future>

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
                                 int max_iter, int row_b, int row_e) {
    std::vector<double> pix;
    for (int i = row_b; i < row_e; i++) {
        for (int j = 0; j < width; j++) {
            double x = -1.5 + 3.0 * j / width;
            double y = 1.5 - 3.0 * i / height;
            complex z(x, y);
            double smooth_color = 0;
            for (int iter = 0; iter < max_iter; iter++) {
                double norm = std::norm(z);
                smooth_color += std::exp(-norm);
                if (norm > 30.0)
                    break;
                z = z * z + c;
            }
            smooth_color /= max_iter;
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
    for (double& d : img) {
        d = std::pow(d, 16);
    }
}

void scale(std::vector<double>& img) {
    double avg = 0;
    for (double d : img) {
        avg += d;
    }
    avg /= img.size();
    for (double& d : img) {
        d /= avg * 15;
    }
}

color palette(double x) {
    static const std::vector<double> CONTROL_X {
        0.0, 0.16, 0.42, 0.6425, 0.8575, 1.0
    };
    static const std::vector<std::tuple<int, int, int>> CONTROL_C {
        std::make_tuple(0, 7, 100),
        std::make_tuple(32, 107, 203),
        std::make_tuple(237, 255, 255),
        std::make_tuple(255, 170, 0),
        std::make_tuple(0, 2, 0),
        std::make_tuple(0, 7, 100)
    };
    
    x -= std::floor(x);
    for (int i = 0; i < CONTROL_X.size() - 1; i++) {
        if (CONTROL_X[i + 1] >= x) {
            double k = (x - CONTROL_X[i]) / (CONTROL_X[i + 1] - CONTROL_X[i]);
            std::tuple<int, int, int> c1 = CONTROL_C[i], c2 = CONTROL_C[i + 1];
            unsigned char r = (1 - k) * std::get<0>(c1) + k * std::get<0>(c2);
            unsigned char g = (1 - k) * std::get<1>(c1) + k * std::get<1>(c2);
            unsigned char b = (1 - k) * std::get<2>(c1) + k * std::get<2>(c2);
            return std::make_tuple(r, g, b);
        }
    }
    throw std::domain_error("invalid palette parameter `x`");
}

Magick::Image julia_set(const complex& c,
                        const int width=DEFAULT_WIDTH,
                        const int height=DEFAULT_HEIGHT,
                        const int num_threads=DEFAULT_NUM_THREADS,
                        const int max_iter=DEFAULT_MAX_ITER) {
    using namespace Magick;

    // Begin multithreaded Julia set calculations
    std::future<std::vector<double>> threads[num_threads];
    for (int tid = 0; tid < num_threads; tid++) {
        int row_b = tid * height / num_threads;
        int row_e = (tid + 1) * height / num_threads;
        threads[tid] = std::async(std::launch::async, julia_pixels,
                                  c, width, height, max_iter, row_b, row_e);
    }

    // Wait on the threads to finish, and combine results to form the image
    std::vector<double> raw_colors;
    for (int tid = 0; tid < num_threads; tid++) {
        threads[tid].wait();
        std::vector<double> partial = threads[tid].get();
        raw_colors.insert(raw_colors.end(), partial.begin(), partial.end());
    }

    // Normalization procedure
    scale(raw_colors);

    // Apply the color palette
    std::vector<unsigned char> pix;
    for (double d : raw_colors) {
        color t = palette(d);
        pix.push_back(std::get<0>(t));
        pix.push_back(std::get<1>(t));
        pix.push_back(std::get<2>(t));
    }
    
    Image image;
    image.read(width, height, "RGB", CharPixel, &pix[0]);
    gaussianBlurImage blur(2, 0.4);
    blur(image);

    return image;
}

int main(int argc, char** argv) {
    using namespace std::chrono;
    using namespace Magick;

    InitializeMagick(*argv);

    // Julia Fractal function:
    // f(z) = z^2 - c
    complex c; // Ex: (-0.221, -0.713);
    std::cout << "Computing the Julia set for f(x) = x^2 + c; c = ";
    std::cin >> c;

    steady_clock::time_point start_time = steady_clock::now();
    Image image = julia_set(c);
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
