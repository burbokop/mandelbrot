#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <ostream>
#include <istream>
#include <vector>

template<typename T>
void serialize(std::ostream &stream, const std::vector<T>& vec) {
    for(const auto v : vec) {
        stream << v;
    }
}

template<typename A, typename B>
void serialize(std::ostream &stream, const std::pair<A, B>& pair) {
    stream << pair.first << pair.second;
}

template<typename T>
void serialize(std::ostream &stream, const T& val) {
    stream << val;
}


class serializer {
    std::ostream *m_s;
public:
    serializer(std::ostream &s) {
        m_s = &s;
    }
    serializer(std::ostream &&s) {
        m_s = &s;
    }
    serializer(const serializer &) = delete;
    void operator=(const serializer &) = delete;

    template <typename T>
    friend serializer &operator <<(serializer& s, const T& value) {
        serialize(*s.m_s, value);
        return s;
    }

    template <typename T>
    friend serializer &operator <<(serializer&& s, const T& value) {
        serialize(*s.m_s, value);
        return s;
    }

};




template<typename T>
void deserialize(std::istream &stream, std::vector<T>& vec) {
    for(auto& v : vec) {
        stream >> v;
    }
}

template<typename A, typename B>
void deserialize(std::istream &stream, std::pair<A, B>& pair) {
    stream >> pair.first >> pair.second;
}

template<typename T>
void deserialize(std::istream &stream, T& val) {
    stream >> val;
}


class deserializer {
    std::istream *m_s;
public:
    deserializer(std::istream &s) {
        m_s = &s;
    }
    deserializer(std::istream &&s) {
        m_s = &s;
    }
    deserializer(const serializer &) = delete;
    void operator=(const serializer &) = delete;

    template <typename T>
    friend deserializer &operator >>(deserializer& s, T& value) {
        deserialize(*s.m_s, value);
        return s;
    }

    template <typename T>
    friend deserializer &operator >>(deserializer&& s, T& value) {
        deserialize(*s.m_s, value);
        return s;
    }

};



#endif // SERIALIZER_H
