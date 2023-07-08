#include "flags.h"

#include <iostream>

Flags Flags::parse(int argc, const char **argv, const std::string &defaultComplexFunctionName)
{
    return e172::FlagParser::parse<Flags>(
               argc,
               argv,
               [&defaultComplexFunctionName](e172::FlagParser &p) -> Flags {
                   return Flags{
                       .testMode = p.flag<bool>(e172::Flag{.shortName = "t",
                                                           .longName = "test",
                                                           .description = "Do optimization test"}),
                       .writeMode = p.flag<bool>(
                           e172::Flag{.shortName = "w",
                                      .longName = "write",
                                      .description = "Write fractal to file"}),
                       .funcList = p.flag<bool>(
                           e172::Flag{.shortName = "l",
                                      .longName = "func-list",
                                      .description
                                      = "Display list of available complex functions"}),
                       .staticDisplay = p.flag<bool>(
                           e172::Flag{.shortName = "s",
                                      .longName = "static-display",
                                      .description = "Display static image"}),
                       .testCount = p.flag(
                           e172::OptFlag<std::size_t>{.shortName = "C",
                                                      .longName = "test-count",
                                                      .description = "Number of tests",
                                                      .defaultVal = 1024}),
                       .function = p.flag(
                           e172::OptFlag<std::string>{.shortName = "f",
                                                      .longName = "func",
                                                      .description = "Specify complex function",
                                                      .defaultVal = defaultComplexFunctionName}),

                       .depth = p.flag(
                           e172::OptFlag<std::size_t>{.shortName = "d",
                                                      .longName = "depth",
                                                      .description = "Fractal per pixel depth",
                                                      .defaultVal = 32}),

                       .colorMask = p.flag(
                           e172::OptFlag<e172::Color>{.shortName = "m",
                                                      .longName = "color-mask",
                                                      .description = "Fractal color mask",
                                                      .defaultVal = 0xffff0000}),
                       .resolution = p.flag(
                           e172::OptFlag<Resolution>{.shortName = "r",
                                                     .longName = "resolution",
                                                     .description = "Fractal resolution",
                                                     .defaultVal = static_cast<std::uint32_t>(
                                                         1024)}),
                       .computeMode = p.flag(e172::OptFlag<FractalView::ComputeMode>{
                           .shortName = "c",
                           .longName = "compute-mode",
                           .description = "Compute mode [cpu=default, cpu-concurent, gpu]",
                           .defaultVal = FractalView::ComputeMode::CPU}),
                       .backgroundColor = p.flag(
                           e172::OptFlag<e172::Color>{.shortName = "b",
                                                      .longName = "background-color",
                                                      .description = "Background color",
                                                      .defaultVal = 0xffffffff}),
                       .graphicsProvider = p.flag(e172::OptFlag<GraphicsProvider>{
                           .shortName = "p",
                           .longName = "graphics-provider",
                           .description = "Graphics provider [sdl=default, console, vulkan]",
                           .defaultVal = GraphicsProvider::SDL}),
                   };
               },
               [](const e172::FlagParser &p) {
                   p.displayErr(std::cerr);
                   std::exit(1);
               },
               [](const e172::FlagParser &p) {
                   p.displayHelp(std::cout);
                   std::exit(0);
               },
               nullptr)
        .value();
}
