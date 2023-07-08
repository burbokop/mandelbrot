#include "flags.h"
#include "fractalview.h"
#include "test.h"
#include <e172/additional.h>
#include <e172/gameapplication.h>
#include <e172/graphics/imageview.h>
#include <e172/impl/console/graphicsprovider.h>
#include <e172/impl/sdl/eventprovider.h>
#include <e172/impl/sdl/graphicsprovider.h>
#include <e172/impl/vulkan/graphicsprovider.h>
#include <e172/math/math.h>
#include <fstream>
#include <iostream>
#include <thread>

bool generateFractalImageFile(std::shared_ptr<e172::AbstractGraphicsProvider> graphicsProvider,
                              const std::string &path,
                              size_t N,
                              e172::MatrixFiller<e172::Color> fractal,
                              e172::Color backgroundColor)
{
    return (graphicsProvider->createImage(N, N, e172::Math::filler(backgroundColor))
            + graphicsProvider->createImage(N, N, fractal)
            ).save(path);
}

bool generateFractalImageFile(std::shared_ptr<e172::AbstractGraphicsProvider> graphicsProvider,
                              std::size_t N,
                              e172::MatrixFiller<e172::Color> fractal,
                              const std::string &description,
                              e172::Color backgroundColor)
{
    return generateFractalImageFile(std::move(graphicsProvider),
                                    "./fractal" + std::to_string(N) + description + ".png",
                                    N,
                                    fractal,
                                    backgroundColor);
}

int main(int argc, const char **argv)
{
    e172::GameApplication app(argc, argv);

    // complex functions
    const std::map<std::string, e172::ComplexFunction<double>> complexFunctions
        = {{"x", [](const auto &x) { return x; }},
           {"sqr", e172::Math::sqr<e172::Complex<double>>},
           {"sin", [](const auto &x) { return std::sin(x); }},
           {"cos", [](const auto &x) { return std::cos(x); }},
           {"sin_sqr", [](const auto &x) { return std::sin(x * x); }},
           {"cos_sqr", [](const auto &x) { return std::cos(x * x); }},
           {"tan_sqr", [](const auto &x) { return std::tan(x * x); }},
           {"asin_sqr", [](const auto &x) { return std::asin(x * x); }},
           {"log_sqr", [](const auto &x) { return std::log(x * x); }},
           {"exp_sqr", [](const auto &x) { return std::exp(x * x); }},
           {"sigm_sqr", [](const auto &x) { return e172::Math::sigm(x * x); }},
           {"floor2_sqr",
            [](const auto &x) {
                const auto t = x * x * 2.;
                return e172::Complex<double>{std::floor(t.real()), std::floor(t.imag())} / 2.;
            }},
           {"floor4_sqr",
            [](const auto &x) {
                const auto t = x * x * 4.;
                return e172::Complex<double>{std::floor(t.real()), std::floor(t.imag())} / 4.;
            }},
           {"floor8_sqr",
            [](const auto &x) {
                const auto t = x * x * 8.;
                return e172::Complex<double>{std::floor(t.real()), std::floor(t.imag())} / 8.;
            }},
           {"floor16_sqr",
            [](const auto &x) {
                const auto t = x * x * 16.;
                return e172::Complex<double>{std::floor(t.real()), std::floor(t.imag())} / 16.;
            }},
           {"floor32_sqr",
            [](const auto &x) {
                const auto t = x * x * 32.;
                return e172::Complex<double>{std::floor(t.real()), std::floor(t.imag())} / 32.;
            }},
           {"sgn_sqr", [](const auto &x) { return e172::Math::sgn(x * x); }}};

    const std::string defaultComplexFunctionName = "sqr";
    const auto flags = Flags::parse(argc, argv, defaultComplexFunctionName);

    if (flags.funcList) {
        std::cout << "Available complex functions:" << std::endl;
        for(const auto& cf : complexFunctions) {
            std::cout << "  " << cf.first << (cf.second.operator bool() ? "" : " (invalid)")
                      << std::endl;
        }
        std::cout << "Default complex function: " << defaultComplexFunctionName << "\n";
        return 0;
    }

    // optimization test
    if (flags.testMode) {
        resolution_test("resolution_test_cache", std::cout, flags.testCount);
        std::this_thread::sleep_for(std::chrono::microseconds(4000));
        return 0;
    }

    const auto complexFunction = [&complexFunctions, &flags] {
        const auto it = complexFunctions.find(flags.function);
        if (it != complexFunctions.end()) {
            return it->second;
        } else {
            std::cerr << "error: Complex function with name '" << flags.function
                      << "' not found.\n";
            std::exit(2);
        }
    }();

    std::map<GraphicsProvider,
             std::function<std::shared_ptr<e172::AbstractGraphicsProvider>(const std::string &)>>
        providerFactories
        = {{GraphicsProvider::SDL,
            [args = app.arguments(),
             &flags](const std::string &title) -> std::shared_ptr<e172::AbstractGraphicsProvider> {
                return std::visit(e172::Overloaded{
                                      [&args, &title](const std::uint32_t res)
                                          -> std::shared_ptr<e172::AbstractGraphicsProvider> {
                                          return std::make_shared<e172::impl::sdl::GraphicsProvider>(
                                              args, title, e172::Vector<std::uint32_t>{res, res});
                                      },
                                      [&args, &title](const ResolutionFullscreen)
                                          -> std::shared_ptr<e172::AbstractGraphicsProvider> {
                                          const auto p
                                              = std::make_shared<e172::impl::sdl::GraphicsProvider>(
                                                  args, title, e172::Vector<std::uint32_t>());
                                          const auto resolution = p->renderer()->screenSize().min();
                                          p->renderer()->setResolution(
                                              e172::Vector(resolution, resolution));
                                          p->renderer()->setFullscreen(true);
                                          return p;
                                      },
                                  },
                                  flags.resolution);
            }},
           {GraphicsProvider::Console,
            [args = app.arguments(),
             &flags](const std::string &title) -> std::shared_ptr<e172::AbstractGraphicsProvider> {
                return std::visit(
                    e172::Overloaded{
                        [&args, &title](const std::uint32_t res)
                            -> std::shared_ptr<e172::AbstractGraphicsProvider> {
                            return std::make_shared<e172::impl::console::GraphicsProvider>(args,
                                                                                           std::cout);
                        },
                        [&args, &title](const ResolutionFullscreen)
                            -> std::shared_ptr<e172::AbstractGraphicsProvider> {
                            const auto p
                                = std::make_shared<e172::impl::console::GraphicsProvider>(args,
                                                                                          std::cout);
                            const auto resolution = p->renderer()->screenSize().min();
                            p->renderer()->setResolution(e172::Vector(resolution, resolution));
                            p->renderer()->setFullscreen(true);
                            return p;
                        },
                    },
                    flags.resolution);
            }},
           {GraphicsProvider::Vulkan,
            [args = app.arguments(),
             &flags](const std::string &title) -> std::shared_ptr<e172::AbstractGraphicsProvider> {
                return std::visit(
                    e172::Overloaded{
                        [&args, &title](const std::uint32_t res)
                            -> std::shared_ptr<e172::AbstractGraphicsProvider> {
                            const auto p = std::make_shared<e172::impl::vulkan::GraphicsProvider>(
                                args);
                            p->renderer()->setResolution(e172::Vector<double>(res, res));
                            return p;
                        },
                        [&args, &title](const ResolutionFullscreen)
                            -> std::shared_ptr<e172::AbstractGraphicsProvider> {
                            const auto p = std::make_shared<e172::impl::vulkan::GraphicsProvider>(
                                args);
                            const auto resolution = p->renderer()->screenSize().min();
                            p->renderer()->setResolution(e172::Vector(resolution, resolution));
                            p->renderer()->setFullscreen(true);
                            return p;
                        },
                    },
                    flags.resolution);
            }}};

    const auto providerFactory = providerFactories.at(flags.graphicsProvider);

    //write flag
    if (flags.writeMode) {
        std::cout << "Write mode." << std::endl;
        std::cout << "Parameters {" << std::endl
                  << "\t\"complex function\": " << flags.function << "," << std::endl
                  << "\t\"resolution\": " << flags.resolution << "," << std::endl
                  << "\t\"color mask\": 0x" << std::hex << flags.colorMask << "," << std::endl
                  << "\t\"background color\": 0x" << std::hex << flags.backgroundColor << ","
                  << std::endl
                  << "\t\"depth\": " << std::dec << flags.depth << "," << std::endl
                  << "\t\"compute mode\": " << FractalView::toString(flags.computeMode) << std::endl
                  << "}" << std::endl
                  << std::endl
                  << "Started. Please wait." << std::endl;

        e172::ElapsedTimer timer;
        if (flags.computeMode == FractalView::ComputeMode::GPU) {
            std::cout << "Warning: graphical compute mode not alloved in write mode. Used simple\n";
        }
        const bool concurent = flags.computeMode == FractalView::ComputeMode::CPUConcurent;
        const auto graphicsProvider = providerFactory({});
        generateFractalImageFile(graphicsProvider,
                                 std::get<std::uint32_t>(flags.resolution),
                                 e172::Math::fractal(flags.depth,
                                                     flags.colorMask,
                                                     complexFunction,
                                                     concurent),
                                 "D" + std::to_string(flags.depth) + "F" + flags.function,
                                 flags.backgroundColor);
        std::cout << "Finished.\nElapsed: " << timer.elapsed() << " ms." << std::endl;
        return 0;
    }

    // static mode
    if (flags.staticDisplay) {
        auto graphicsProvider = providerFactory("Static fractal view (" + flags.function + ")");
        std::cout << "Parameters {" << std::endl
                  << "\t\"complex function\": " << flags.function << "," << std::endl
                  << "\t\"resolution\": " << flags.resolution << "," << std::endl
                  << "\t\"color mask\": 0x" << std::hex << flags.colorMask << "," << std::endl
                  << "\t\"background color\": 0x" << std::hex << flags.backgroundColor << ","
                  << std::endl
                  << "\t\"depth\": " << std::dec << flags.depth << "," << std::endl
                  << "\t\"concurent\": " << FractalView::toString(flags.computeMode) << std::endl
                  << "}" << std::endl;

        if (flags.computeMode == FractalView::ComputeMode::GPU) {
            std::cout << "Warning: graphical compute mode not alloved in static mode. Used simple\n";
        }
        bool concurent = flags.computeMode == FractalView::ComputeMode::CPUConcurent;

        app.setGraphicsProvider(graphicsProvider);
        app.setEventProvider(std::make_shared<e172::impl::sdl::EventProvider>());

        app.addEntity(e172::FactoryMeta::make<e172::ImageView>(
            graphicsProvider->createImage(std::get<std::uint32_t>(flags.resolution),
                                          std::get<std::uint32_t>(flags.resolution),
                                          e172::Math::filler(flags.backgroundColor))
            + graphicsProvider->createImage(std::get<std::uint32_t>(flags.resolution),
                                            std::get<std::uint32_t>(flags.resolution),
                                            e172::Math::fractal(flags.depth,
                                                                flags.colorMask,
                                                                complexFunction,
                                                                concurent))));
        return app.exec();
    }

    //default mode
    {
        auto graphicsProvider = providerFactory("Fractal view (" + flags.function + ")");

        app.setGraphicsProvider(graphicsProvider);
        app.setEventProvider(std::make_shared<e172::impl::sdl::EventProvider>());
        app.setRenderInterval(1000 / 30);

        app.addEntity(e172::FactoryMeta::make<FractalView>(std::get<std::uint32_t>(flags.resolution),
                                                           flags.depth,
                                                           flags.colorMask,
                                                           flags.backgroundColor,
                                                           complexFunction,
                                                           flags.computeMode));

        return app.exec();
    }
}
