#pragma once

#include <e172/entity.h>
#include <e172/graphics/abstractrenderer.h>
#include <e172/math/math.h>
#include <e172/time/elapsedtimer.h>
#include <e172/utility/flagparser.h>

class FractalView : public e172::Entity {
public:
    enum class ComputeMode { CPU, CPUConcurent, GPU };

    static std::string toString(ComputeMode computeMode);

    template<typename T>
    static T expRoof(const T &v)
    {
        if(v < 2) {
            return 2;
        } else if(v <= 4) {
            return 4;
        } else if(v <= 8) {
            return 8;
        } else if(v <= 16) {
            return 16;
        } else if(v <= 32) {
            return 32;
        } else if(v <= 64) {
            return 64;
        } else if(v <= 128) {
            return 128;
        } else if(v <= 256) {
            return 256;
        } else if(v <= 512) {
            return 512;
        } else {
            return 1024;
        }
    }

    FractalView(
        e172::FactoryMeta &&meta,
        std::size_t resolution,
        std::size_t depthMultiplier,
        e172::Color colorMask,
        e172::Color backgroundColor,
        const e172::ComplexFunction<double> &function = e172::Math::sqr<e172::Complex<double>>,
        ComputeMode computeMode = ComputeMode::CPU);

    ComputeMode computeMode() const;

    // Entity interface
public:
    void proceed(e172::Context *, e172::EventHandler *eventHandler) override;
    void render(e172::Context *, e172::AbstractRenderer *renderer) override;

private:
    e172::ComplexFunction<double> m_function;
    e172::Color m_colorMask, m_backgroundColor;

    ComputeMode m_computeMode;
    size_t m_resolution;
    size_t m_depthMultiplier;

    e172::Vector<double> m_offset;
    double m_zoom = 0.5;

    size_t m_updateResolutionBegin;
    size_t m_updateResolution;

    std::vector<e172::ElapsedTimer> m_inputTimers;
};

inline e172::Either<e172::FlagParseError, FractalView::ComputeMode> operator>>(
    e172::RawFlagValue raw, e172::TypeTag<FractalView::ComputeMode>)
{
    if (raw.str == "cpu") {
        return e172::Right(FractalView::ComputeMode::CPU);
    } else if (raw.str == "cpu-concurent") {
        return e172::Right(FractalView::ComputeMode::CPUConcurent);
    } else if (raw.str == "gpu") {
        return e172::Right(FractalView::ComputeMode::GPU);
    } else {
        return e172::Left(e172::FlagParseError::EnumValueNotFound);
    }
}
