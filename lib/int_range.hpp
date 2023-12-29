#pragma once

#include <ostream>

namespace simpleConfig {

    struct int_range {
        int min = 0;
        int max = 0;

        int_range() = default;

        int_range(int l, int h) : min(l), max(h)
        {}

        int_range &operator=(const int_range &o) = default;

        friend std::ostream & operator<<(std::ostream &strm, const int_range& ir) {
            strm << "[" << ir.min << ", " << ir.max << "]";
            return strm;
        }

        bool operator==(const int_range &o) {
            return (min == o.min && max == o.max);
        }

    };

}