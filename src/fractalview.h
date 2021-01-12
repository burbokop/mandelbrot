#ifndef FRACTALVIEW_H
#define FRACTALVIEW_H

#include <src/entity.h>
#include <src/graphics/abstractrenderer.h>
#include <src/math/math.h>
#include <src/time/elapsedtimer.h>


class FractalView : public e172::Entity {
    e172::ComplexFunction m_function;
    e172::Color m_colorMask, m_backgroundColor;
    bool m_concurent;
    size_t m_resolution;
    size_t m_depthMultiplier;

    e172::Vector offset;
    double zoom = 0.5;

    size_t m_updateResolutionBegin;
    size_t m_updateResolution;

    std::vector<e172::ElapsedTimer> inputTimers;
public:
    template<typename T>
    static T exp_roof(const T& v) {
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

    FractalView(size_t resolution, size_t depthMultiplier, e172::Color colorMask, e172::Color backgroundColor, const e172::ComplexFunction& function = e172::Math::sqr<e172::Complex>, bool concurent = true);

    // Entity interface
public:
    virtual void proceed(e172::Context *, e172::AbstractEventHandler *eventHandler) override;
    virtual void render(e172::AbstractRenderer *renderer) override;
};

#endif // FRACTALVIEW_H
