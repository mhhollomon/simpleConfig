#pragma once

#include <string_view>
#include <ostream>
#include <list>

namespace simpleConfig {

    struct parse_loc {
        std::string_view sv;
        int offset = 0;
        int line = 0;
    };

    struct error_info {
        std::string message;
        parse_loc loc;
        std::string src = "Unk";

        error_info() = default;
        error_info(const std::string &mesg, const parse_loc &l) :
            message{mesg}, loc{l} {}

        error_info(const std::string &mesg, const parse_loc &l, const std::string &source) :
            message{mesg}, loc{l}, src{source} {}

        friend std::ostream & operator<<(std::ostream &strm, const error_info& ei) {
            strm << "line " << ei.loc.line << " : " << ei.src << ": " << ei.message << "\n";
            return strm;
        }
    };

    struct error_list {
        std::string_view src;
        std::list<error_info> errors;

        int count() const {
            return static_cast<int>(errors.size());
        }

        bool empty() const { return errors.empty(); }

        void add(const std::string &mesg, const parse_loc &l) {
            errors.emplace_back(mesg, l);
        }

        void add(const std::string &mesg, const parse_loc &l, const std::string &source) {
            errors.emplace_back(mesg, l, source);
        }

        friend std::ostream & operator<<(std::ostream &strm, const error_list & el){
            for (auto &e : el.errors) {
                strm << e;
            }
            return strm;
        }
    };

}