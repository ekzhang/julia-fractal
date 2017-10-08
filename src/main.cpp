#include "fractal.h"

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
