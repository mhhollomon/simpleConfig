#pragma once

#include "parser_utils.hpp"

namespace simpleConfig {
    
    struct ErrorReporter {

        error_list &errors;
        int error_count = 0;
        const std::string name;

        ErrorReporter(const std::string &n, error_list &errlist ) :
            errors{errlist},
            name{n}
        {}

        /***********************************************************
         * Error utilities
         ***********************************************************/

        bool has_errors() { return error_count > 0; }

        void record_error(const std::string &msg, const parse_loc &l) {
            error_count += 1;
            errors.add(msg, l, name);
        }


    };
}