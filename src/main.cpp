#include <src/additional.h>
#include <src/gameapplication.h>
#include <src/sdleventhandler.h>
#include <src/sdlgraphicsprovider.h>
#include <src/math/math.h>
#include <src/graphics/imageview.h>
#include <src/utility/testing.h>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <thread>
#include <execution>
#include "test.h"

const size_t width = 600;
const size_t height = 600;


bool generateMandelbrotImage(e172::AbstractGraphicsProvider *graphicsProvider, const std::string &path, size_t N, e172::MatrixFiller<e172::Color> fractal) {
    //if(std::filesystem::exists(path))
    //    return true;

    return (graphicsProvider->createImage(N, N, e172::Math::filler(0xffffffff))
            + graphicsProvider->createImage(N, N, fractal)
            ).save(path);
}

bool generateMandelbrotImage(e172::AbstractGraphicsProvider *graphicsProvider, size_t N, e172::MatrixFiller<e172::Color> fractal) {
    return generateMandelbrotImage(graphicsProvider, "./fractal" + std::to_string(N) + ".png", N, fractal);
}

e172::Image composeWithMandelbrot(e172::AbstractGraphicsProvider *graphicsProvider, const e172::Image &image, const e172::Vector &offset, size_t depth, e172::Color mask) {
    const auto size = image.size().min() - offset.max();
    const auto mandelbrotImage = graphicsProvider->createImage(size, size, e172::Math::fractal<e172::Color>(depth, mask));
    return image.blit(mandelbrotImage, offset.x(), offset.y()).fragment(offset.x(), offset.y(), size, size);
}

int main(int argc, char **argv) {
    size_t depth = 32;
    e172::Color colorMask = 0xffff8800;
    size_t resolution = 1024;
    bool useMultithreading = true;

    if(argc > 1) {
        if(std::string(argv[1]) == "--test") {
            size_t test_count = 1024;
            if(argc > 2)
                test_count = std::stoul(argv[2]);
            resolution_test("resolution_test_cache", std::cout, test_count);
            std::this_thread::sleep_for(std::chrono::microseconds(4000));
            return 0;
        } else if(std::string(argv[1]) == "--help") {
            std::cout << "args: [depth] [color mask] [resolution] [use multithreading]\n\nadditional options:\n\t--test [test count] - do test (do not run any apps while testing)\n\t--help - display help\n";
            return 0;
        } else {
            depth = std::stoul(argv[1]);
        }
    }

    if(argc > 2)
        colorMask = std::stoul(argv[2], nullptr, 16);

    if(argc > 3)
        resolution = std::stoul(argv[3]);

    if(argc > 4)
        useMultithreading = std::stoul(argv[4]);

    e172::GameApplication app(argc, argv);
    SDLGraphicsProvider graphicsProvider(app.arguments(), "Mandelbrot", width, height);
    SDLEventHandler eventHandler;

    app.setGraphicsProvider(&graphicsProvider);
    app.setEventHandler(&eventHandler);

    std::cout << "Default Mode Enabled. For more info print --help\n";

    std::cout << "Parameters: resolution: " << resolution << ", color: " << std::hex << colorMask << ", depth: " << std::dec << depth << ", multithreading: " << useMultithreading << "\nStarted.\n";
    e172::ElapsedTimer timer;
    generateMandelbrotImage(&graphicsProvider, resolution, e172::Math::fractal(depth, colorMask, [](const auto& x){ return std::tan(x * x); }, useMultithreading));
    std::cout << "Finished.\nElapsed: " << timer.elapsed() << " ms\n";

    return 0;
    return app.exec();
}
