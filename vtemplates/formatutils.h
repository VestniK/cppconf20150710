#pragma once

#include <cstdio>
#include <string>
#include <vector>

class Format {
public:
    template<typename... A>
    std::string operator() (const A &... a) const {
        const size_t len = std::snprintf(nullptr, 0, m_fmt, a...) + 1;
        std::vector<char> buf;
        buf.resize(len);
        std::snprintf(buf.data(), buf.size(), m_fmt, a...);
        return buf.data();
    }

    const char *format() const {return m_fmt;}
private:
    const char *m_fmt;

    constexpr Format(const char *fmt): m_fmt(fmt) {}
    friend constexpr Format operator "" _fmt (const char *, size_t);
};

constexpr inline
Format operator "" _fmt (const char *fmt, size_t) {
    return {fmt};
}
