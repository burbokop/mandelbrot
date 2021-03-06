#include <src/additional.h>
#include <src/gameapplication.h>
#include <src/sdleventhandler.h>
#include <src/sdlgraphicsprovider.h>
#include <src/math/math.h>
#include <src/graphics/imageview.h>
#include <src/utility/testing.h>

#include <iostream>
#include <fstream>
#include <thread>
#include "fractalview.h"
#include "test.h"

using namespace std::complex_literals;

bool generateFractalImageFile(e172::AbstractGraphicsProvider *graphicsProvider, const std::string &path, size_t N, e172::MatrixFiller<e172::Color> fractal, e172::Color backgroundColor) {
    return (graphicsProvider->createImage(N, N, e172::Math::filler(backgroundColor))
            + graphicsProvider->createImage(N, N, fractal)
            ).save(path);
}

bool generateFractalImageFile(e172::AbstractGraphicsProvider *graphicsProvider, size_t N, e172::MatrixFiller<e172::Color> fractal, const std::string& description, e172::Color backgroundColor) {
    return generateFractalImageFile(graphicsProvider, "./fractal" + std::to_string(N) + description + ".png", N, fractal, backgroundColor);
}

int main(int argc, char **argv) {
    e172::GameApplication app(argc, argv);

    //constants
    const std::map<std::string, e172::ComplexFunction> complexFunctions = {
        { "x", [](const auto& x){ return x; } },
        { "sqr", e172::Math::sqr<e172::Complex> },
        { "sin", [](const auto& x){ return std::sin(x); } },
        { "cos", [](const auto& x){ return std::cos(x); } },
        { "sin_sqr", [](const auto& x){ return std::sin(x * x); } },
        { "cos_sqr", [](const auto& x){ return std::cos(x * x); } },
        { "tan_sqr", [](const auto& x){ return std::tan(x * x); } },
        { "asin_sqr", [](const auto& x){ return std::asin(x * x); } },
        { "log_sqr", [](const auto& x){ return std::log(x * x); } },
        { "exp_sqr", [](const auto& x){ return std::exp(x * x); } },
        { "sigm_sqr", [](const auto& x){ return e172::Math::sigm(x * x); } },
        { "floor2_sqr", [](const auto& x){ const auto t = x * x * 2.; return e172::Complex { std::floor(t.real()), std::floor(t.imag()) } / 2.; } },
        { "floor4_sqr", [](const auto& x){ const auto t = x * x * 4.; return e172::Complex { std::floor(t.real()), std::floor(t.imag()) } / 4.; } },
        { "floor8_sqr", [](const auto& x){ const auto t = x * x * 8.; return e172::Complex { std::floor(t.real()), std::floor(t.imag()) } / 8.; } },
        { "floor16_sqr", [](const auto& x){ const auto t = x * x * 16.; return e172::Complex { std::floor(t.real()), std::floor(t.imag()) } / 16.; } },
        { "floor32_sqr", [](const auto& x){ const auto t = x * x * 32.; return e172::Complex { std::floor(t.real()), std::floor(t.imag()) } / 32.; } },
        { "sgn_sqr", [](const auto& x){ return e172::Math::sgn(x * x); } }
    };

    const std::string defaultComplexFunctionName = "sqr";

    //global variables
    std::pair<std::string, e172::ComplexFunction> currentComplexFunction = *complexFunctions.find(defaultComplexFunctionName);
    size_t depth;
    e172::Color colorMask;
    e172::Color backgroundColor;
    size_t resolution;
    bool concurent;

    //flags registration
    app.registerBoolFlag  ( "-t", "--test",             "Do optimization test"                        );
    app.registerValueFlag ( "-C", "--test-count",       "Specify test count"                          );
    app.registerBoolFlag  ( "-w", "--write",            "Write fractal to file"                       );
    app.registerValueFlag ( "-f", "--func",             "Specify complex function"                    );
    app.registerBoolFlag  ( "-l", "--func-list",        "Display list of available complex functions" );
    app.registerBoolFlag  ( "-s", "--static-display",   "Display static image"                        );

    app.registerValueFlag ( "-d", "--depth",            "Fractal per pixel depth"                     );
    app.registerValueFlag ( "-m", "--color-mask",       "Fractal color mask"                          );
    app.registerValueFlag ( "-r", "--resolution",       "Fractal resolution"                          );
    app.registerBoolFlag  ( "-c", "--concurent",        "Use multiple threads"                        );
    app.registerValueFlag ( "-b", "--background-color", "Background color"                            );

    //func list flag
    if(app.containsFlag("-l")) {
        std::cout << "Available complex functions:\n";
        for(const auto& cf : complexFunctions) {
            std::cout << "  " << cf.first << (cf.second.operator bool() ? "" : " (invalid)") << "\n";
        }
        std::cout << "Default complex function: " << defaultComplexFunctionName << "\n";
        return 0;
    }

    //optimization test flag
    if(app.containsFlag("-t")) {
        auto test_count = app.flag("-C");
        if(test_count.isNull())
            test_count = 1024;

        resolution_test("resolution_test_cache", std::cout, test_count.toSize_t());
        std::this_thread::sleep_for(std::chrono::microseconds(4000));
        return 0;
    }

    //function specification flag
    {
        const auto fn = app.flag("-f").toString();
        if(fn.size() > 0) {
            const auto it = complexFunctions.find(fn);
            if(it != complexFunctions.end()) {
                currentComplexFunction = *it;
            } else {
                std::cerr << "Error: Complex function with name '" << fn << "' not found.\n";
                return -1;
            }
        }
    }

    //depth flag
    {
        const auto depthFlag = app.flag("-d");
        if(depthFlag.isNull()) {
            depth = 32;
        } else {
            depth = depthFlag.toSize_t();
        }
    }

    //color mask flag
    {
        const auto cmFlag = app.flag("-m");
        if(cmFlag.isNull()) {
            colorMask = 0xffff0000;
        } else {
            colorMask = std::stoul(cmFlag.toString(), nullptr, 16);
        }
    }

    //background color flag
    {
        const auto flag = app.flag("-b");
        if(flag.isNull()) {
            backgroundColor = 0xffffffff;
        } else {
            std::cout << "bg flag: " << flag << "\n";
            backgroundColor = std::stoul(flag.toString(), nullptr, 16);
        }
    }

    //resolution flag
    {
        auto rFlag = app.flag("-r");
        if(rFlag.isNull()) {
            resolution = 1024;
        } else {
            resolution = rFlag.toSize_t();
        }
    }

    //concurent flag
    {
        concurent = app.containsFlag("-c");
    }

    //write flag
    if(app.containsFlag("-w")) {
        SDLGraphicsProvider gp(app.arguments(), {}, 0, 0);
        std::cout << "Write mode.\n";
        std::cout
                << "Parameters {\n"
                << "\t\"complex function\": " << currentComplexFunction.first << ",\n"
                << "\t\"resolution\": " << resolution << ",\n"
                << "\t\"color mask\": 0x" << std::hex << colorMask << ",\n"
                << "\t\"background color\": 0x" << std::hex << backgroundColor << ",\n"
                << "\t\"depth\": " << std::dec << depth << ",\n"
                << "\t\"concurent\": " << concurent << "\n"
                << "}\n\n"
                << "Started. Please wait.\n";
        e172::ElapsedTimer timer;
        generateFractalImageFile(&gp, resolution, e172::Math::fractal(depth, colorMask, currentComplexFunction.second, concurent), "D" + std::to_string(depth) + "F" + currentComplexFunction.first, backgroundColor);
        std::cout << "Finished.\nElapsed: " << timer.elapsed() << " ms.\n";
        return 0;
    }

    //static mode
    if(app.containsFlag("-s")) {
        std::cout
                << "Parameters {\n"
                << "\t\"complex function\": " << currentComplexFunction.first << ",\n"
                << "\t\"resolution\": " << resolution << ",\n"
                << "\t\"color mask\": 0x" << std::hex << colorMask << ",\n"
                << "\t\"background color\": 0x" << std::hex << backgroundColor << ",\n"
                << "\t\"depth\": " << std::dec << depth << ",\n"
                << "\t\"concurent\": " << concurent << "\n"
                << "}\n";

        SDLGraphicsProvider graphicsProvider(app.arguments(), ("Static fractal view (" + currentComplexFunction.first + ")").c_str(), resolution, resolution);
        SDLEventHandler eventHandler;

        app.setGraphicsProvider(&graphicsProvider);
        app.setEventHandler(&eventHandler);

        e172::ImageView fractalView
                = graphicsProvider.createImage(resolution, resolution, e172::Math::filler(backgroundColor))
                + graphicsProvider.createImage(resolution, resolution, e172::Math::fractal(depth, colorMask, currentComplexFunction.second, concurent));

        app.addEntity(&fractalView);

        return app.exec();
    }

    //default mode
    {
        SDLGraphicsProvider graphicsProvider(app.arguments(), ("Fractal view (" + currentComplexFunction.first + ")").c_str(), resolution, resolution);
        SDLEventHandler eventHandler;

        app.setGraphicsProvider(&graphicsProvider);
        app.setEventHandler(&eventHandler);

        FractalView fractalView(resolution, depth, colorMask, backgroundColor, currentComplexFunction.second, concurent);
        app.addEntity(&fractalView);

        return app.exec();
    }
}
