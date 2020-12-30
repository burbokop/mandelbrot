#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <e172/src/entity.h>

class Mandelbrot : public e172::Entity {
    double zoom = 0.5;
    size_t N = 600;
    size_t limit = 1;
public:
    Mandelbrot();

    // Entity interface
public:
    virtual void proceed(e172::Context *context, e172::AbstractEventHandler *eventHandler) override;
    virtual void render(e172::AbstractRenderer *renderer) override;
};

#endif // MANDELBROT_H
