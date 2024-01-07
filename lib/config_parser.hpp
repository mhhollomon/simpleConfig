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

            skip();
            ENTER;

            // For the first one, it can be anything
            do {
                if (peek() == '{') {
                    consume(1);
                    auto *element = new Setting(VT::GROUP);
                    if (not parse_group(element)) {
                        RETURN_B(false);
                    }
                    skip();
                    if (! match_char('}')) {
                        record_error("Did not see close brace for subgroup");
                        RETURN_B(false);
                    }
                    consume(1);
                    setting->add_child(*element);
                    std::cout << "=== Array with group child now has array type = "
                        << int(setting->array_type()) << "\n";
                    break;
                }

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

                if (setting->array_type() == VT::GROUP) {
                    if (peek() != '{') {
                        break;
                    }
                    consume(1);
                    auto *element = new Setting(VT::GROUP);
                    if (not parse_group(element)) {
                        RETURN_B(false);
                    }
                    skip();
                    if (! match_char('}')) {
                        record_error("Did not see close brace for subgroup");
                        RETURN_B(false);
                    }
                    consume(1);
                    setting->add_child(*element);

                } else if (setting->array_type() == VT::BOOL) {
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
                record_error("All values in an array must be the same type");
                RETURN_B(false);
            }

            RETURN_B(true);

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
            skip();
            ENTER;

            auto name = match_name();
            if ( ! name ) {
                RETURN_B(false);
            }

            skip();

            if (not match_chars(0, ":=")) {
                record_error("Expecting : or = after setting name");
                RETURN_B(false);
            }
            consume(1);

            auto new_setting = parent->create_child(*name);

            if (!new_setting) {
                record_error("Setting named "s + *name + " already defined in this context");
                RETURN_B(false);
            }

            skip();
            bool retval = parse_setting_value(new_setting);
            RETURN_B(retval);
            
        }

        //##############   parse_group #####################

        bool parse_group(Setting * parent) {
            skip();
            ENTER;

            bool at_least_one = false;
            while (1) {
                if (!parse_setting(parent)) break;
                at_least_one = true;

                if (match_chars(0, ";,")) {
                    consume(1);
                }
                skip();
            }

            RETURN_B(at_least_one);
        }

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