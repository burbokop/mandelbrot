#include "test.h"

#include <e172/conversion.h>
#include <e172/math/differentiator.h>
#include <e172/math/discretizer.h>
#include <e172/math/intergrator.h>
#include <e172/math/math.h>
#include <e172/variant.h>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace e172 {
class Testing
{
public:
    static std::vector<double> compare_test(size_t count,
                                            const std::function<void()> &f0,
                                            const std::function<void()> &f1,
                                            bool verbose = false);

    template<typename T, typename Iterator>
    static std::vector<double> compare_test(Iterator begin,
                                            Iterator end,
                                            const std::function<void(const T &)> &f0,
                                            const std::function<void(const T &)> &f1,
                                            bool verbose = false)
    {
        std::list<double> result;
        for (Iterator it = begin; it != end; ++it) {
            const auto t0 = std::chrono::system_clock::now();
            f0(*it);
            const auto t1 = std::chrono::system_clock::now();
            f1(*it);
            const auto t2 = std::chrono::system_clock::now();
            const auto c = (t2 - t1).count();
            if (c != 0) {
                result.push_back(double((t1 - t0).count()) / double(c));
            } else {
                result.push_back(std::numeric_limits<double>::max());
            }
            if (verbose)
                std::cout << *it << ":" << result.back() << "\n";
        }
        return std::vector<double>(result.begin(), result.end());
    }

    template<typename T>
    static std::vector<T> make_set(size_t first, T max, const std::function<T(size_t)> &f)
    {
        std::list<T> result;
        for (size_t i = first;; ++i) {
            result.push_back(f(i));
            if (max < result.back()) {
                break;
            }
        }
        return std::vector<T>(result.begin(), result.end());
    }
};
} // namespace e172

void depth_test(std::ostream &out) {
    out << "depth_test()\n";
    uint32_t mask = 0xffffffff;
    constexpr size_t N = 512;
    const auto x = e172::Testing::make_set<double>(2, 64, [](size_t x){ return x; });
    const auto y = e172::Testing::compare_test<size_t>(x.begin(), x.end(), [mask](size_t depth) {
        uint32_t a[N * N];
        e172::Math::fractal(depth, mask)(N, N, a);
    }, [mask](size_t depth) {
        uint32_t a[N * N];
        e172::Math::fractal(depth, mask)(N, N, a);
    }, true);

    out << "average:" << e172::Math::average(y) << "\n";
    plt::make_plot(x, y);
    std::system("gnuplot ./plot.plg");





}






void resolution_test(const std::string &cache_path, std::ostream &out, size_t test_count) {
    const char test_data[] = { 0x67, 0x12, 0x46, 0x66, 0x67, 0x0b, 0x20, 0x68, 0x20, 0x5f, 0x66, 0x64, 0x73, 0x10, 0x2c, 0x3f, 0x11, 0x46, 0x2d, 0x05, 0x20, 0xb, 0x03, 0x0c, 0x5, 0x7e };
    std::cout << "test_data: " << e172::Variant(e172::VariantMap { { "data", test_data } }).toJson() << "\n";




    out << "resolution_test()\n";
    constexpr uint32_t mask = 0xffffffff;
    constexpr size_t depth = 32;
    constexpr size_t multiplier = 8;

    const auto file = cache_path + std::to_string(test_count) + ".json";

    plt::sequence_pair xy;
    if(std::filesystem::exists(file)) {
        std::ifstream stream(file);
        std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        xy = e172::convert_to<plt::sequence_pair>(e172::Variant::fromJson(str));
        std::cout << "loaded from cache\n";
    } else {
        xy.first = e172::Testing::make_set<double>(2, test_count, [](size_t x){ return x * multiplier; });
        xy.second = e172::Testing::compare_test<size_t>(xy.first.begin(), xy.first.end(), [](size_t N) {
            uint32_t a[N * N];
            e172::Math::fractal(depth, mask, e172::Math::sqr<e172::Complex<double>>, false)(N, N, a);
        }, [](size_t N) {
            uint32_t a[N * N];
            e172::Math::fractal(depth, mask, e172::Math::sqr<e172::Complex<double>>, true)(N, N, a);
        }, true);
        std::fstream() << std::string();

        std::ofstream(file, std::ios::out)
                << e172::convert_to<e172::Variant>(xy).toJson();
    }

    const auto integrated_xy = e172::intergate<1>(e172::discretize<0, 1>(std::pair { xy.first, xy.second }, multiplier));
    const auto differentiated_y = e172::differentiateVec({xy.first, xy.second});
    const auto differentiated_smoosed_xy = e172::intergate<1>(e172::discretize<0, 1>(std::pair { xy.first, differentiated_y }, multiplier));

    const auto mr = plt::make_plots(integrated_xy.first, { integrated_xy.second, differentiated_smoosed_xy.second }, { 1 });
    std::system(("gnuplot " + mr.first->file_path()).c_str());
}

plt::plot plt::make_plot(const std::vector<double> &x, const std::vector<double> &y) {
    const auto csv_file = std::make_shared<tmp_file>("csv");
    const auto plot_file = std::make_shared<tmp_file>("plg");

    std::stringstream stream;
    for(size_t i = 0; i < std::min(x.size(), y.size()); ++i) {
        stream << x[i] << ", " << y[i] << "\n";
    }

    std::string plot = "plot '" + csv_file->file_path() + "' title 'main' with lines, 1 with lines lt 1 title 'y = 1'\npause -1\n";
    const auto csv = stream.str();

    plot_file->w(plot.c_str(), plot.size());
    csv_file->w(csv.c_str(), csv.size());
    return { plot_file, csv_file };
}


plt::plot plt::make_plots(const plt::sequence &x, const plt::sequence_vector &values, const std::vector<double> &horisontalLines, int pause) {
    const auto csv_file = std::make_shared<tmp_file>("csv");
    const auto plot_file = std::make_shared<tmp_file>("plg");

    std::stringstream stream;
    const auto count = std::min(x.size(), std::min_element(values.begin(), values.end(), [](const sequence& a, const sequence& b){ return a.size() < b.size(); })->size());
    for(size_t i = 0; i < count; ++i) {
        stream << x[i] << ", ";

        for(size_t j = 0; j < values.size(); ++j) {
            stream << values[j][i];
            if(j != values.size() - 1) {
                stream << ", ";
            }
        }
        stream << "\n";
    }

    const auto cfp = "'" + csv_file->file_path() + "'";
    std::string plot = "plot ";
    for(size_t i = 0; i < horisontalLines.size(); ++i) {
        const auto v = std::to_string(horisontalLines[i]);
        plot += v + " with lines lt 1 title 'y = " + v + "'";
        if(i < values.size() + horisontalLines.size() - 1) {
            plot += ", ";
        }
    }
    for(size_t i = 0; i < values.size(); ++i) {
        plot += cfp + " using " + std::to_string(i + 2) + " title 'chart" + std::to_string(i) + "' with lines";
        if(i < values.size() - 1) {
            plot += ", ";
        }
    }
    plot += "\n";
    if(pause != 0) {
        plot += "pause " + std::to_string(pause) + "\n";
    }
    std::cout << "plot text:\n" << plot << "\n";
    const auto csv = stream.str();

    plot_file->w(plot.c_str(), plot.size());
    csv_file->w(csv.c_str(), csv.size());
    return { plot_file, csv_file };

}



plt::tmp_file::tmp_file(const std::string &suffix) {
    size_t suffixSize;
    if(suffix.size() > 0) {
        if(suffix[0] == '.') {
            m_file_path = "/tmp/tmp_XXXXXX" + suffix;
            suffixSize = suffix.size();
        } else {
            m_file_path = "/tmp/tmp_XXXXXX." + suffix;
            suffixSize = suffix.size() + 1;
        }
    } else {
        m_file_path = "/tmp/tmp_XXXXXX";
        suffixSize = 0;
    }
    m_fd = mkstemps(m_file_path.data(), suffixSize);
}

plt::tmp_file::~tmp_file() {
    unlink(m_file_path.c_str());
}

ssize_t plt::tmp_file::w(const void *ptr, size_t size) {
    return write(m_fd, ptr, size);
}

std::string plt::tmp_file::file_path() const {
    return m_file_path;
}

