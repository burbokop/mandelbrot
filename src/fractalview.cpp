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
    inputTimers = { 64, 64, 64 };
    m_updateResolutionBegin = m_resolution / 4;
    m_updateResolution = m_updateResolutionBegin;
}

void FractalView::proceed(e172::Context *, e172::AbstractEventHandler *eventHandler) {
    if(inputTimers[0].check(eventHandler->keyHolded(e172::ScancodeMinus))) {
        zoom *= 0.9;
        m_updateResolution = m_updateResolutionBegin;
    } else if(inputTimers[0].check(eventHandler->keyHolded(e172::ScancodeEquals))) {
        zoom /= 0.9;
        m_updateResolution = m_updateResolutionBegin;
    }

    if(inputTimers[1].check(eventHandler->keyHolded(e172::ScancodeLeft))) {
        offset.decrementX(0.1 / zoom);
        m_updateResolution = m_updateResolutionBegin;
    } else if(inputTimers[1].check(eventHandler->keyHolded(e172::ScancodeRight))) {
        offset.incrementX(0.1 / zoom);
        m_updateResolution = m_updateResolutionBegin;
    }

    if(inputTimers[2].check(eventHandler->keyHolded(e172::ScancodeUp))) {
        offset.decrementY(0.1 / zoom);
        m_updateResolution = m_updateResolutionBegin;
    } else if(inputTimers[2].check(eventHandler->keyHolded(e172::ScancodeDown))) {
        offset.incrementY(0.1 / zoom);
        m_updateResolution = m_updateResolutionBegin;
    }
}
#include <iostream>
void FractalView::render(e172::AbstractRenderer *renderer) {
    if(m_resolution < m_updateResolutionBegin) {
        return;
    }

    if(m_updateResolution <= m_resolution) {
        renderer->setAutoClear(false);
        renderer->fill(0);
        const size_t depth = exp_roof(m_depthMultiplier * zoom);
        if(m_updateResolution > 0) {
            const size_t w = m_resolution;
            const size_t h = m_resolution;

            const auto bitmap = renderer->bitmap();
            const auto bmw = renderer->resolution().size_tX();
            const auto bms = bmw * renderer->resolution().size_tY();


            const auto find_rem = [this](size_t x) -> size_t { return x - x % (m_resolution / m_updateResolution); };

            const auto exec_line = [bitmap, bmw, bms, this, h, w, depth, find_rem](size_t y) {
                const auto rem_y = find_rem(y);
                for(size_t x = 0; x < w; ++x) {
                    const auto rem_x = find_rem(x);
                    auto i = y * bmw + x;
                    auto ri = rem_y * bmw + rem_x;
                    if(i >= 0 && i < bms) {
                        if(rem_x == x && rem_y == y) {
                            const auto &value = e172::Vector(double(x) / double(w) * 2 - 1, double(y) / double(h) * 2 - 1) / zoom + offset;
                            const auto level = e172::Math::fractalLevel(value.toComplex(), depth, m_function);
                            bitmap[i] = e172::blendPixels(e172::Color(double(level) / double(depth) * m_colorMask), m_backgroundColor);
                        } else {
                            bitmap[i] = bitmap[ri];
                        }
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
        }

        const auto xyz_string = "{ " + std::to_string(offset.x()) + ", " + std::to_string(offset.y()) + ", " + std::to_string(zoom) + " }";
        const auto depth_string =  "\nDepth: " + std::to_string(depth);
        if(xyz_string.size() > 0) {
            renderer->drawString(xyz_string + depth_string, { 8, 8. }, 0xffffff, e172::TextFormat::fromFontSize(m_resolution / xyz_string.size()));
        }
        m_updateResolution *= 2;
    }
}
