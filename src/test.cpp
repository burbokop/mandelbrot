#include "test.h"

#include <ostream>
#include <e172/src/utility/testing.h>
#include <e172/src/math/math.h>
#include <src/math/intergrator.h>
#include <src/math/discretizer.h>
#include <src/math/differentiator.h>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <src/variant.h>
#include <e172/src/conversion.h>

void depth_test(std::ostream &out) {
    out << "depth_test()\n";
    uint32_t mask = 0xffffffff;
    constexpr size_t N = 512;
    const auto x = e172::Testing::make_set<double>(2, 64, [](size_t x){ return x; });
    const auto y = e172::Testing::compare_test<size_t>(x.begin(), x.end(), [mask](size_t depth) {
        uint32_t a[N * N];
        e172::Math::fractal<uint32_t>(depth, mask)(N, N, a);
    }, [mask](size_t depth) {
        uint32_t a[N * N];
        e172::Math::fractal(depth, mask)(N, N, a);
    }, true);

    out << "average:" << e172::Math::average(y) << "\n";
    plt::make_plot(x, y);
    std::system("gnuplot ./plot.plg");
}






void resolution_test(const std::string &cache_path, std::ostream &out, size_t test_count) {
    out << "resolution_test()\n";
    constexpr uint32_t mask = 0xffffffff;
    constexpr size_t depth = 32;
    constexpr size_t multiplier = 8;

    const auto file = cache_path + std::to_string(test_count) + ".json";
    std::vector<double> x;
    std::vector<double> y;
    if(std::filesystem::exists(file)) {
        std::ifstream stream(file);
        std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        const auto data = e172::Variant::fromJson(str);

        std::cout << "loaded from cache\n";

        auto map = data.toMap();

        x = convert_to<decltype (x)>(map["first"].toList());
        y = convert_to<decltype (y)>(map["second"].toList());
    } else {
        x = e172::Testing::make_set<double>(2, test_count, [](size_t x){ return x * multiplier; });
        y = e172::Testing::compare_test<size_t>(x.begin(), x.end(), [](size_t N) {
            uint32_t a[N * N];
            e172::Math::fractal<uint32_t>(depth, mask, e172::Math::sqr<e172::Complex>, false)(N, N, a);
        }, [](size_t N) {
            uint32_t a[N * N];
            e172::Math::fractal<uint32_t>(depth, mask, e172::Math::sqr<e172::Complex>, true)(N, N, a);
        }, true);
        std::fstream() << std::string();

        std::ofstream(file, std::ios::out)
                << e172::Variant(e172::toVariantMap(std::pair { e172::toVariantList(x), e172::toVariantList(y) })).toJson();
    }

    const auto integrated_xy = e172::intergate<1>(e172::discretize<0, 1>(std::pair { x, y }, multiplier));
    const auto differentiated_y = e172::differentiate_vec({ x, y });
    const auto differentiated_smoosed_xy = e172::intergate<1>(e172::discretize<0, 1>(std::pair { x, differentiated_y }, multiplier));

    const auto r0 = plt::make_plot(x, y);
    const auto r1 = plt::make_plot(integrated_xy);
    const auto r2 = plt::make_plot(x, differentiated_y);
    const auto r3 = plt::make_plot(differentiated_smoosed_xy);
    const auto mr = plt::make_plots(integrated_xy.first, { integrated_xy.second, differentiated_smoosed_xy.second });

    std::system(("gnuplot " + r0.first->file_path()).c_str());
    std::system(("gnuplot " + r1.first->file_path()).c_str());
    std::system(("gnuplot " + r2.first->file_path()).c_str());
    std::system(("gnuplot " + r3.first->file_path()).c_str());


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


plt::plot plt::make_plots(const plt::sequence &x, const plt::sequence_vector &values) {
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

    std::string plot = "plot '" + csv_file->file_path() + "' title 'main' with lines, 1 with lines lt 1 title 'y = 1'\npause -1\n";
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

