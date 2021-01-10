#ifndef TEST_H
#define TEST_H

#include <memory>
#include <vector>
#include <e172/src/variant.h>

void depth_test(std::ostream& out);

void resolution_test(const std::string &cache_path, std::ostream& out);



template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref>: std::true_type {};


template <typename T>
T convert_to(const e172::VariantList &list) {
    if constexpr(is_specialization<T, std::vector>::value) {
        typedef typename T::value_type val_t;
        if constexpr(std::is_integral<val_t>::value || std::is_same<val_t, double>::value || std::is_same<val_t, float>::value) {
            T result;
            result.reserve(list.size());
            for(const auto& a : list) {
                result.push_back(a.toNumber<val_t>());
            }
            return result;
        } else {
            return {};
        }
    } else {
        return {};
    }
}



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

std::pair<std::shared_ptr<tmp_file>, std::shared_ptr<tmp_file>> write_plot(const std::vector<double> &x, const std::vector<double> &y);
inline std::pair<std::shared_ptr<tmp_file>, std::shared_ptr<tmp_file>> write_plot(const std::pair<std::vector<double>, std::vector<double>> &values) {
    return write_plot(values.first, values.second);
}


#endif // TEST_H
