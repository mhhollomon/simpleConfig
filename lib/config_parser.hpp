#pragma once

#include "parser_base.hpp"

#include "value_type.hpp"

#include "simpleConfig.hpp"

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
        
        //##############   match_string_value  ###############
        // Supports parsing strings "next" to each other as a single
        // string
        std::optional<std::string> match_string_value() {
            if (!match_char('"')) return std::nullopt;

            // consume the opening quotes
            consume(1);
            std::stringstream buf{};

            bool stop = false;
            while(not stop and not eoi()) {
                char c = peek();
                switch (c) {
                    case '\0' :
                        //must be eoi;
                        stop = true;
                        break;
                    case '\\' :
                        if (match_chars(1, "\\fnrtx\"")) {
                            switch(peek(1)) {
                            case 'f' :
                                buf << '\f';
                                consume(2);
                                break;
                            case 'n' :
                                buf << '\n';
                                consume(2);
                                break;
                            case '"' :
                                buf << '"';
                                consume(2);
                                break;
                            case 'r' :
                                buf << '\r';
                                consume(2);
                                break;
                            case '\\' :
                                buf << '\\';
                                consume(2);
                                break;
                            case 't' :
                                buf << '\t';
                                consume(2);
                                break;
                            case 'x' :
                                if (match_chars(2, "0123456789abcdefABCDEF") and
                                        match_chars(3, "0123456789abcdefABCDEF")) {
                                    char x = (peek(2) - '0') * 16 + (peek(3) - '0');
                                    buf << x;
                                    consume(4);
                                } else {
                                    record_error("Bad hex escape in string");
                                    consume(2); // just to keep us going
                                }
                                break;
                            }
                        } else {
                            record_error("Unrecognized escape sequence in string");
                            consume(1);
                        }
                    case '\n' :
                        record_error("Unterminated string");
                        return std::nullopt;
                        break;
                    case '"' :
                        stop = true;
                        consume(1);
                        break;
                    default:
                        buf << c;
                        // this will be slow - rethink.
                        consume(1);
                        break;
                    }
                if (stop) {
                    skip();
                    if (match_char('"')) {
                        stop = false;
                        consume(1);
                    }
                }
            }

            return buf.str();

        }
        //##############   match_scalar_value  ###############

        bool match_scalar_value(Setting *parent) {

            auto bv = match_bool_value();
            if (bv) {
                parent->set_value(*bv);
                return true;
            }
            
            auto lv = match_integer_value();
            if (lv) {
                parent->set_value(*lv);
                return true;
            }

            auto dv = match_double_value();
            if (dv) {
                parent->set_value(*dv);
                return true;
            }

            auto sv = match_string_value();
            if (sv) {
                parent->set_value(*sv);
                return true;
            }


            return false;
        }


        //##############   parse_list     ##############
        bool parse_list(Setting* setting) {
            skip();
            while(1) {
                if (peek() == ')') {
                    return true;
                }
                Setting *child = setting->create_child();
                if (!parse_setting_value(child)) {
                    return false;
                }
                skip();
                if (match_chars(0, ";,")) {
                    consume(1);
                    skip();
                }
            }

            // Never should get here. At some point parse_setting_value()
            // will fail and we'll exit inside the loop
            return false;
        }

        //##############   parse_array    ##############
        bool parse_array(Setting * setting) {

            // For the first one, it can be anything
            do {
                auto bv = match_bool_value();
                if (bv) {
                    setting->add_child(*bv);
                    break;
                }
                
                auto lv = match_integer_value();
                if (lv) {
                    setting->add_child(*lv);
                    break;
                }

                auto dv = match_double_value();
                if (dv) {
                    setting->add_child(*dv);
                    break;
                }

                auto sv = match_string_value();
                if (sv) {
                    setting->add_child(*sv);
                    break;
                }
            } while(false);


            while (1) {
                skip();
                if (match_chars(0, ";,")) {
                    consume(1);
                    skip();
                }
                if (setting->array_type() == VT::BOOL) {
                    auto bv = match_bool_value();
                    if (bv) {
                        setting->add_child(*bv);
                    } else {
                        break;
                    }

                } else if (setting->array_type() == VT::INTEGER) {
                    auto lv = match_integer_value();
                    if (lv) {
                        setting->add_child(*lv);
                    } else {
                        break;
                    }

                } else if (setting->array_type() == VT::FLOAT) {
                    auto dv = match_double_value();
                    if (dv) {
                        setting->add_child(*dv);
                    } else {
                        break;
                    }
                } else if (setting->array_type() == VT::STRING) {

                    auto sv = match_string_value();
                    if (sv) {
                        setting->add_child(*sv);
                    } else {
                        break;
                    }
                } else {
                    throw std::runtime_error("Unexpected array_type");
                }
            }

            Setting tester{};

            if (match_scalar_value(&tester)) {
                record_error("All values in an array must be the same scalar type");
                return false;
            }

            return true;

        }

        //##############   parse_setting_value ##############

        bool parse_setting_value(Setting * setting, bool record_failure=true) {
            if (peek() == '{') {
                consume(1);
                skip();
                setting->make_group();
                parse_group(setting);
                skip();
                if (peek(0) != '}') {
                    record_error("Didn't find close of setting group");
                    return false;
                }
                consume(1);
            } else if (peek() == '(') {
                consume(1);
                skip();
                setting->make_list();
                parse_list(setting);
                skip();
                if (peek() != ')') {
                    record_error("Didn't find close of setting list");
                    return false;
                }
                consume(1);
            } else if (peek() == '[') {
                consume(1);
                skip();
                setting->make_array();
                parse_array(setting);
                skip();
                if (peek() != ']') {
                    record_error("Didn't find close of value array");
                    return false;
                }
                consume(1);
            } else if (! match_scalar_value(setting)) {
                if (record_failure) record_error("Expecting a value");
                return false;
            }

            return true;
        }
        //##############   setting ##########################

        bool parse_setting(Setting * parent) {

            auto name = match_name();
            if ( ! name ) {
                return false;
            }

            skip();

            if (not match_chars(0, ":=")) {
                record_error("Expecting : or = after setting name");
                return false;
            }
            consume(1);

            auto new_setting = parent->create_child(*name);

            if (!new_setting) {
                record_error("Setting named "s + *name + " already defined in this context");
                return false;
            }

            skip();

            return parse_setting_value(new_setting);
            
        }

        //##############   parse_group #####################

        bool parse_group(Setting * parent) {
            bool at_least_one = false;
            while (1) {
                if (!parse_setting(parent)) break;
                at_least_one = true;

                if (match_chars(0, ";,")) {
                    consume(1);
                }
                skip();
            }

            return at_least_one;
        }

        //##############   do_parse  #####################
        
        bool do_parse() {

            error_count = 0;

            skip();

            parse_group(setting);

            if (! eoi()) {
                record_error("Not at end of input!");
                return false;
            }

            return not has_errors();
        }

    };


}