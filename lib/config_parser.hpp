#pragma once

#include "parser_base.hpp"

#include "value_type.hpp"
#include "setting.hpp"

#include <string_view>
#include <optional>
#include <sstream>


namespace simpleConfig {

    using VT = ValType;


    struct Parser : public ParserBase {

        Setting *setting;

        Parser(std::string_view _src, Setting *s, error_list &errlist) :
            ParserBase(_src, "Config"s, errlist),
            setting{s}
        {}
        

        //##############   do_parse  #####################
        
        bool do_parse() {
            ENTER;

            error_count = 0;

            skip();

            parse_group(setting);

            if (! eoi()) {
                record_error("Not at end of input!");
                RETURN_B(false);
            }

            RETURN_B(not has_errors());
        }

    };


}