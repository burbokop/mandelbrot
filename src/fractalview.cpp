#include "fractalview.h"

#include <boost/compute/algorithm/transform.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/core.hpp>
#include <e172/debug.h>
#include <e172/eventhandler.h>
#include <e172/functional/metafunction.h>
#include <e172/utility/defer.h>
#include <exception>
#include <execution>
#include <iostream>

FractalView::ComputeMode FractalView::computeMode() const {
    return m_computeMode;
}

BOOST_COMPUTE_FUNCTION(int, add_four, (int x), {
                           return x + 4;
                       });

std::string FractalView::toString(FractalView::ComputeMode computeMode) {
    if (computeMode == ComputeMode::CPU) {
        return "cpu";
    } else if (computeMode == ComputeMode::CPUConcurent) {
        return "cpu-concurent";
    } else if (computeMode == ComputeMode::GPU) {
        return "gpu";
    } else {
        return "undefined";
    }
}

FractalView::FractalView(e172::FactoryMeta &&meta,
                         std::size_t resolution,
                         std::size_t depthMultiplier,
                         e172::Color colorMask,
                         e172::Color backgroundColor,
                         const e172::ComplexFunction<double> &function,
                         ComputeMode computeMode)
    : e172::Entity(std::forward<e172::FactoryMeta>(meta))
    , m_resolution(resolution)
    , m_depthMultiplier(depthMultiplier)
    , m_colorMask(colorMask)
    , m_backgroundColor(backgroundColor)
    , m_function(function)
    , m_computeMode(computeMode)
    , m_inputTimers({64, 64, 64})
    , m_updateResolutionBegin(m_resolution / 64)
    , m_updateResolution(m_updateResolutionBegin)
{
    if (computeMode == ComputeMode::GPU) {
        boost::compute::device device = boost::compute::system::default_device();
        boost::compute::context context(device);
        boost::compute::command_queue queue(context, device);

        int host_data[] = { 1, 2, 3, 4, 5 };

        boost::compute::vector<int> device_vector(5, context);
        boost::compute::copy(host_data, host_data + 5, device_vector.begin(), queue);


        boost::compute::transform(device_vector.begin(),
                                  device_vector.end(),
                                  device_vector.begin(),
                                  add_four,
                                  queue
                                  );

        std::vector<int> host_vector(5);
        boost::compute::copy(device_vector.begin(), device_vector.end(), host_vector.begin(), queue);

        for(auto v : host_vector) {
            e172::Debug::print(":", v);
        }

        e172::Debug::print("device:", device.name(), "\n");
    }
}

void FractalView::proceed(e172::Context *, e172::EventHandler *eventHandler) {
    if (m_inputTimers[0].check(eventHandler->keyHolded(e172::ScancodeMinus))) {
        m_zoom *= 0.9;
        m_updateResolution = m_updateResolutionBegin;
    } else if (m_inputTimers[0].check(eventHandler->keyHolded(e172::ScancodeEquals))) {
        m_zoom /= 0.9;
        m_updateResolution = m_updateResolutionBegin;
    }

    if (m_inputTimers[1].check(eventHandler->keyHolded(e172::ScancodeLeft))) {
        m_offset.decrementX(0.1 / m_zoom);
        m_updateResolution = m_updateResolutionBegin;
    } else if (m_inputTimers[1].check(eventHandler->keyHolded(e172::ScancodeRight))) {
        m_offset.incrementX(0.1 / m_zoom);
        m_updateResolution = m_updateResolutionBegin;
    }

    if (m_inputTimers[2].check(eventHandler->keyHolded(e172::ScancodeUp))) {
        m_offset.decrementY(0.1 / m_zoom);
        m_updateResolution = m_updateResolutionBegin;
    } else if (m_inputTimers[2].check(eventHandler->keyHolded(e172::ScancodeDown))) {
        m_offset.incrementY(0.1 / m_zoom);
        m_updateResolution = m_updateResolutionBegin;
    }
}

void FractalView::render(e172::Context *, e172::AbstractRenderer *renderer)
{
    if(m_resolution < m_updateResolutionBegin) {
        return;
    }

    if(m_updateResolution <= m_resolution) {
        renderer->setAutoClear(false);
        //renderer->fill(0);
        const size_t depth = expRoof(m_depthMultiplier * m_zoom);
        //const size_t depth = m_depthMultiplier * (zoom > 1 ? std::sqrt(zoom) : zoom);

        const auto deteriorationCoef = m_resolution / m_updateResolution;
        if (deteriorationCoef == 0) {
            std::cout << "deterioration_coef is 0. " << std::to_string(m_resolution) << " / " << std::to_string(m_updateResolution) << std::endl;
            return;
        }
        if(m_updateResolution > 0) {
            const size_t w = m_resolution;
            const size_t h = m_resolution;

            renderer->modifyBitmap([this, renderer, w, h, depth, deteriorationCoef](
                                       e172::Color *bitmap) {
                const auto bmw = renderer->resolution().size_tX();
                const auto bms = bmw * renderer->resolution().size_tY();

                const auto find_rem = [deteriorationCoef](size_t x) -> size_t {
                    return x - x % deteriorationCoef;
                };

                const auto exec_line = [bitmap, bmw, bms, this, h, w, depth, find_rem](size_t y) {
                    const auto rem_y = find_rem(y);
                    for(size_t x = 0; x < w; ++x) {
                        const auto rem_x = find_rem(x);
                        auto i = y * bmw + x;
                        auto ri = rem_y * bmw + rem_x;
                        if(i >= 0 && i < bms) {
                            if(rem_x == x && rem_y == y) {
                                const auto &value = e172::Vector(double(x) / double(w) * 2 - 1,
                                                                 double(y) / double(h) * 2 - 1)
                                                        / m_zoom
                                                    + m_offset;
                                const auto level = e172::Math::fractalLevel(value.toComplex(), depth, m_function);
                                const auto coef = double(level) / double(depth);

                                const auto c = e172::Color(m_colorMask * coef);
                                if(i == 0) {
                                    std::cout << "D: " << std::dec << depth << ", L: " << level << ", DD: " << (double(level) / double(depth)) << ", c: " << std::hex << c << "\n";
                                }

                                bitmap[i] = e172::blend(c, m_backgroundColor);
                            } else {
                                bitmap[i] = bitmap[ri];
                            }
                        }
                    }
                };
                std::cout << "ccc: " << toString(m_computeMode) << "\n";

                if (m_computeMode == ComputeMode::GPU) {
                    todo();
                } else if (m_computeMode == ComputeMode::CPUConcurent) {
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
            });
        }

        const auto xyz_string = "{ " + std::to_string(m_offset.x()) + ", "
                                + std::to_string(m_offset.y()) + ", " + std::to_string(m_zoom)
                                + " }";
        const auto depth_string = "\nDepth: " + std::to_string(depth)
                                  + " Deterioration: " + std::to_string(deteriorationCoef);
        if(xyz_string.size() > 0) {
            std::cout << "r/ss: " << m_resolution << " / " << xyz_string.size() << "\n";
            renderer->drawString(xyz_string + depth_string, { 8, 8. }, 0xffffff, e172::TextFormat::fromFontSize(m_resolution / xyz_string.size()));
        }
        m_updateResolution *= 2;
    }
}
