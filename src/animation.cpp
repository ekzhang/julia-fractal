#include "fractal.h"

int main(int argc, char** argv) {
    using namespace std::chrono;
    using namespace Magick;

    InitializeMagick(*argv);

    const double PI = std::atan2(0, -1);
    const int NUM_FRAMES = 360;
    const int FPS = 20;

    steady_clock::time_point start_time = steady_clock::now();

    std::vector<Image> frames;
    for (double frame = 0; frame < NUM_FRAMES; frame++) {
        std::cout << "Currently on frame " << frame + 1 << "/" << NUM_FRAMES << "..." << std::endl;
        double arg = frame * 2 * PI / NUM_FRAMES;
        complex c = std::polar(0.8785, arg);
        Image im = julia_set(c, 512, 512, scale);
        im.animationDelay(100.0 / FPS);
        frames.push_back(im);
    }

    steady_clock::time_point end_time = steady_clock::now();
    long long time_millis = duration_cast<milliseconds>(end_time - start_time).count();
    std::cout << "Finished generating image, took " << time_millis << "ms." << std::endl;

    Magick::writeImages(frames.begin(), frames.end(), "output/animation.gif");
    return 0;
}
