#include <chrono>
#include <complex>
#include <iostream>
#include <string>
#include <future>

#include <Magick++.h>

typedef std::complex<double> complex;

std::vector<unsigned char> julia_pixels(complex c, int width, int height,
                                        int row_b, int row_e) {
    std::vector<unsigned char> pix;
    for (int i = row_b; i < row_e; i++) {
        for (int j = 0; j < width; j++) {
            double x = -1.4 + 2.8 * j / width;
            double y = 1.4 - 2.8 * i / height;
            complex z(x, y);
            int iter;
            for (iter = 0; iter < 255; iter++) {
                z = z * z + c;
                if (std::norm(z) > 2.0)
                    break;
            }
            pix.push_back(iter);
            pix.push_back(iter);
            pix.push_back(iter);
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
    
    // Width and height of the output image
    const int width = 1920 * 2;
    const int height = 1080 * 2;

    // Number of threads to use when multithreading
    const int num_threads = 4;
    std::future<std::vector<unsigned char>> threads[num_threads];

    steady_clock::time_point start_time = steady_clock::now();

    for (int tid = 0; tid < num_threads; tid++) {
        int row_b = tid * height / num_threads;
        int row_e = (tid + 1) * height / num_threads;
        threads[tid] = std::async(std::launch::async, julia_pixels,
                                  c, width, height, row_b, row_e);
    }

    std::vector<unsigned char> pix;
    for (int tid = 0; tid < num_threads; tid++) {
        threads[tid].wait();
        std::vector<unsigned char> partial = threads[tid].get();
        pix.insert(pix.end(), partial.begin(), partial.end());
    }
    
    Image image;
    image.read(width, height, "RGB", CharPixel, &pix[0]);
    gaussianBlurImage blur(2, 0.4);
    blur(image);

    steady_clock::time_point end_time = steady_clock::now();
    long long time_millis = duration_cast<milliseconds>(end_time - start_time).count();
    std::cout << "Finished generating image, took " << time_millis << "ms." << std::endl;
    
    std::string c_pretty = std::to_string(c.real()) + (c.imag() >= 0 ? "+" : "") + std::to_string(c.imag()) + "j";
    std::string out_filename = "output/" + c_pretty + ".png";
    image.write(out_filename);
    system(("open " + out_filename).c_str());
    return 0;
}
