#include "fractalview.h"

#include <src/abstracteventhandler.h>
#include <execution>



FractalView::FractalView(size_t resolution, size_t depthMultiplier, e172::Color colorMask, e172::Color backgroundColor, const e172::ComplexFunction &function, bool concurent) {
    m_resolution = resolution;
    m_depthMultiplier = depthMultiplier;
    m_colorMask = colorMask;
    m_backgroundColor = backgroundColor;
    m_function = function;
    m_concurent = concurent;
}

void FractalView::proceed(e172::Context *, e172::AbstractEventHandler *eventHandler) {
    if(eventHandler->keyHolded(e172::ScancodeMinus)) {
        zoom *= 0.99;
        doUpdate = true;
    } else if(eventHandler->keyHolded(e172::ScancodeEquals)) {
        zoom /= 0.99;
        doUpdate = true;
    }

    if(eventHandler->keyHolded(e172::ScancodeLeft)) {
        offset.decrementX(0.1 / zoom);
        doUpdate = true;
    } else if(eventHandler->keyHolded(e172::ScancodeRight)) {
        offset.incrementX(0.1 / zoom);
        doUpdate = true;
    }
    if(eventHandler->keyHolded(e172::ScancodeUp)) {
        offset.decrementY(0.1 / zoom);
        doUpdate = true;
    } else if(eventHandler->keyHolded(e172::ScancodeDown)) {
        offset.incrementY(0.1 / zoom);
        doUpdate = true;
    }

    if(eventHandler->keyHolded(e172::Scancode1)) {
        m_resolution /= 2;
        doUpdate = true;
    } else if(eventHandler->keyHolded(e172::Scancode2)) {
        m_resolution *= 2;
        doUpdate = true;
    }
}

void FractalView::render(e172::AbstractRenderer *renderer) {
    if(doUpdate) {
        renderer->setAutoClear(false);
        renderer->fill(0);
        const size_t w = m_resolution;
        const size_t h = m_resolution;

        const auto bitmap = renderer->bitmap();
        const auto bmw = renderer->resolution().size_tX();
        const auto bms = bmw * renderer->resolution().size_tY();
        const size_t depth = exp_roof(m_depthMultiplier * zoom);

        const auto exec_line = [bitmap, bmw, bms, this, h, w, depth](size_t y) {
            for(size_t x = 0; x < w; ++x) {
                const auto i = y * bmw + x;
                if(i >= 0 && i < bms) {
                    const auto &value = e172::Vector(double(x) / double(w) * 2 - 1, double(y) / double(h) * 2 - 1) / zoom + offset;
                    const auto level = e172::Math::fractalLevel(value.toComplex(), depth, m_function);
                    bitmap[i] = e172::blendPixels(e172::Color(double(level) / double(depth) * m_colorMask), m_backgroundColor);
                }
            }
        };

        if(m_concurent) {
            std::vector<size_t> job(h);
            for(size_t y = 0; y < h; ++y) {
                job[y] = y;
            }
            std::for_each(std::execution::par_unseq, job.begin(), job.end(), exec_line);
        } else {
            for(size_t y = 0; y < h; ++y) {
                exec_line(y);
            }
        }

        const auto xyz_string = "{ " + std::to_string(offset.x()) + ", " + std::to_string(offset.y()) + ", " + std::to_string(zoom) + " }";
        const auto depth_string =  "\nDepth: " + std::to_string(depth);
        if(xyz_string.size() > 0) {
            renderer->drawString(xyz_string + depth_string, { 8, 8. }, 0xffffff, e172::TextFormat::fromFontSize(m_resolution / xyz_string.size()));
        }
        doUpdate = false;
    }
}
