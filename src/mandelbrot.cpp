#include "mandelbrot.h"

#include <src/abstracteventhandler.h>
#include <src/debug.h>

#include <src/graphics/abstractrenderer.h>

template <typename T>
T mandel_func(T x, T c) { return x * x + c; }

bool is_in_mandel(e172::Complex c, size_t limit = 256) {
    e172::Complex x = { 0, 0 };
    while (std::abs(x) < 2) {
        x = mandel_func(x, c);
        if(limit-- <= 0) {
            return false;
        }
    }
    return true;
}

bool vec_is_in_mandel(e172::Vector c, size_t limit = 256) {
    return is_in_mandel(c.toComplex(), limit);
}


Mandelbrot::Mandelbrot() {}

void Mandelbrot::proceed(e172::Context *context, e172::AbstractEventHandler *eventHandler) {
    if(eventHandler->keyHolded(e172::ScancodeUp)) {
        zoom *= 1.01;
    }
    if(eventHandler->keyHolded(e172::ScancodeDown)) {
        zoom /= 1.01;
    }
    if(eventHandler->keyHolded(e172::ScancodeLeft)) {
        if(N > 0)
            N -= 1;
    }
    if(eventHandler->keyHolded(e172::ScancodeRight)) {
        N += 1;
    }
    if(eventHandler->keyHolded(e172::ScancodeN)) {
        if(limit > 0)
            limit -= 1;
    }
    if(eventHandler->keyHolded(e172::ScancodeM)) {
        limit += 1;
    }
}

void Mandelbrot::render(e172::AbstractRenderer *renderer) {
    if(N > 0) {
        for(size_t y = 0; y < N; ++y) {
            for(size_t x = 0; x < N; ++x) {
                const auto c = e172::Vector { double(x) / double(N) - 0.5, double(y) / double(N) - 0.5 } * 4;

                if(vec_is_in_mandel(c, limit)) {
                    renderer->drawPixelShifted(c * renderer->resolution().min() * 0.5 * zoom, 0x00ff00);
                }
            }
        }
    }

    renderer->drawStringShifted("N:" + std::to_string(N), { 20, 260 }, 0xffff00);
    renderer->drawStringShifted("L:" + std::to_string(limit) + " Z:" + std::to_string(zoom), { 20, 280 }, 0xffff00);
}
