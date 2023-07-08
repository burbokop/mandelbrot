#pragma once

#include "fractalview.h"

#include <cstddef>
#include <e172/graphics/color.h>
#include <e172/utility/flagparser.h>
#include <string>
#include <variant>

struct ResolutionFullscreen
{};

using Resolution = std::variant<std::uint32_t, ResolutionFullscreen>;

inline std::ostream &operator<<(std::ostream &stream, const Resolution &r)
{
    return std::visit(e172::Overloaded{
                          [&stream](const std::uint32_t res) -> std::ostream & {
                              return stream << res;
                          },
                          [&stream](const ResolutionFullscreen) -> std::ostream & {
                              return stream << "fullscreen";
                          },
                      },
                      r);
}

inline e172::Either<e172::FlagParseError, Resolution> operator>>(e172::RawFlagValue raw,
                                                                 e172::TypeTag<Resolution>)
{
    if (raw.str == "fullscreen") {
        return e172::Right<Resolution>(ResolutionFullscreen{});
    } else {
        return (raw >> e172::TypeTag<std::uint32_t>{}).map<Resolution>([](auto v) { return v; });
    }
}

enum class GraphicsProvider { SDL, Console, Vulkan };

inline e172::Either<e172::FlagParseError, GraphicsProvider> operator>>(
    e172::RawFlagValue raw, e172::TypeTag<GraphicsProvider>)
{
    if (raw.str == "sdl") {
        return e172::Right(GraphicsProvider::SDL);
    } else if (raw.str == "console") {
        return e172::Right(GraphicsProvider::Console);
    } else if (raw.str == "vulkan") {
        return e172::Right(GraphicsProvider::Vulkan);
    } else {
        return e172::Left(e172::FlagParseError::EnumValueNotFound);
    }
}

struct Flags
{
    bool testMode;
    bool writeMode;
    bool funcList;
    bool staticDisplay;
    std::size_t testCount;
    std::string function;
    std::size_t depth;
    e172::Color colorMask;
    Resolution resolution;
    FractalView::ComputeMode computeMode;
    e172::Color backgroundColor;
    GraphicsProvider graphicsProvider;

    static Flags parse(int argc, const char **argv, const std::string &defaultComplexFunctionName);
};
