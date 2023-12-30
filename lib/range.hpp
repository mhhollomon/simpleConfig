#pragma once

#include <ostream>

namespace simpleConfig {

    template <typename T = long>
    struct Range {
        T min = 0;
        T max = 0;

        bool limited = false;

        Range() = default;

        Range(T l, T h) : min(l), max(h)
        {}

        Range &operator=(const Range &o) = default;

        friend std::ostream & operator<<(std::ostream &strm, const Range& ir) {
            strm << "[" << ir.min << ", " << ir.max << "]";
            return strm;
        }

        bool operator==(const Range &o) {
            return (min == o.min && max == o.max);
        }

    };

}