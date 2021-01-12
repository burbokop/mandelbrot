#ifndef TEST_H
#define TEST_H

#include <memory>
#include <vector>
#include <e172/src/variant.h>

void depth_test(std::ostream& out);

void resolution_test(const std::string &cache_path, std::ostream& out, size_t test_count = 1024);


namespace plt {

typedef std::vector<double> sequence;
typedef std::pair<sequence, sequence> sequence_pair;
typedef std::vector<sequence> sequence_vector;

class tmp_file {
    int m_fd = 0;
    std::string m_file_path;
public:
    tmp_file(const std::string &suffix);
    tmp_file(const tmp_file&) = delete;
    ~tmp_file();
    ssize_t w(const void *ptr, size_t size);
    std::string file_path() const;
};

typedef std::pair<std::shared_ptr<tmp_file>, std::shared_ptr<tmp_file>> plot;

plot make_plot(const sequence &x, const sequence &y);
inline plot make_plot(const sequence_pair &xy) {
    return make_plot(xy.first, xy.second);
}

plot make_plots(const sequence &x, const sequence_vector &values, const std::vector<double> &horisontalLines = std::vector<double>(), int pause = -1);

}

#endif // TEST_H
