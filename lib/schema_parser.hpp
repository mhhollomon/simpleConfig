#pragma once

#include "parser_base.hpp"

#include "value_type.hpp"

#include "schema_node.hpp"
#include "validator.hpp"

#include <string>
#include <string_view>
#include <charconv>
#include <optional>
#include <sstream>
#include <map>
#include <memory>
#include <iostream>

using namespace std::literals::string_literals;

using VT = simpleConfig::ValType;

namespace simpleConfig {

    struct type_info {
        ValType vtype = ValType::NONE;
        bool normal = true;
    };

    std::map<std::string, type_info>type_map = {
        {"int", {ValType::INTEGER, true}},
        {"bool", {ValType::BOOL, true}},
        {"float", {ValType::FLOAT, true}},
        {"string", {ValType::STRING, true}},
        {"any", {ValType::ANY, true}},
        {"group", {ValType::GROUP, false}},
        {"array", {ValType::ARRAY, false}},
        {"list", {ValType::LIST, false}}
    };


    struct SchemaParser : public ParserBase {

        SchemaNode *schema;

        SchemaParser(std::string_view src, SchemaNode* _schema, error_list & errlist) : 
            ParserBase(src, "Schema"s, errlist),
            schema{_schema}
        { }

        //############## parse_int_range #########################
        std::optional<Range<long>> parse_int_range(bool length_mode) {
            skip();
            ENTER;
            Range<long> retval{};

            if ( ! expect_char('['))
                RETURN_NULLOPT;

            auto min = match_integer_value();
            if (!min) {
                record_error("Invalid range entry.");
                RETURN_NULLOPT
            }

            retval.min = *min;
            retval.max = *min;
            retval.limited = true;

            optional_sep();

            auto max = match_integer_value();
            if (max) {
                skip();
                optional_sep();
                retval.max = *max;
                skip();
            } else if (! length_mode) {
                record_error("Missing max value");
                RETURN_NULLOPT;
            }

            if (!expect_char(']')) {
                record_error("Missing ']' closing the range spec.");
                RETURN_NULLOPT;
            }

            if (length_mode && retval.min < 0) {
                record_error("Min range entry cannot be less than 0.");
                RETURN_NULLOPT
            }

            if (length_mode && retval.max <= 0) {
                record_error("Max range entry cannot be less than or equal to 0.");
                RETURN_NULLOPT
            }

            if (retval.min > retval.max) {
                record_error("Min must be less than or equal to Max in range");
                RETURN_NULLOPT
            }


            RETURN(retval);            
        }

        //############## parse_float_range #########################
        std::optional<Range<double>> parse_float_range() {
            skip();
            ENTER;
            Range<double> retval{};

            if ( ! expect_char('['))
                RETURN_NULLOPT;

            auto min = match_double_value();
            if (!min) {
                record_error("Invalid range entry.");
                RETURN_NULLOPT
            }

            retval.min = *min;
            retval.max = *min;
            retval.limited = true;

            optional_sep();

            auto max = match_double_value();
            if (max) {
                skip();
                optional_sep();
                retval.max = *max;
                skip();
            } else  {
                record_error("Missing max value");
                RETURN_NULLOPT;
            }

            if (!expect_char(']')) {
                record_error("Missing ']' closing the range spec.");
                RETURN_NULLOPT;
            }

            if (retval.min > retval.max) {
                record_error("Min must be less than or equal to Max in range");
                RETURN_NULLOPT
            }


            RETURN(retval);            
        }
        //##############  parse_typename #######################

        bool parse_typename(SchemaNode * keyspec, bool extended = false) {
            skip();
            ENTER;

            ValType vtype = VT::NONE;


            for (auto const &iter : type_map ){
                if (not extended and not iter.second.normal) {
                    // std::cout << "Skipping type name '"s + iter.first + "'\n";
                    continue;
                }
                //std::cout << "Trying type name '"s + iter.first + "'\n";

                if (match_string(iter.first)) {
                    //std::cout << "--- Found '" + iter.first +"'\n";
                    vtype = iter.second.vtype;
                    break;
                }
            }

            if (vtype == VT::NONE) {
                record_error("Invalid or missing type constraint for key '"s + 
                        keyspec->name + "'");
                return false;
            }

            if (! check_string_end()) {
                record_error("Invalid type constraint for key specifier");
                return false;
            }

            if (keyspec->vtype == VT::ARRAY) {
                keyspec->array_type = vtype;
            } else {
                keyspec->vtype = vtype;
            }
            return true;
        }

        //############## parse_constraint ##################

        bool parse_constraint(SchemaNode* parent) {
            skip();
            ENTER;

            if (peek() == '{') {
                consume(1);
                skip();
                if (! parse_extended_list(parent)) {
                    return false;
                }
                skip();
                if (peek() == '}') {
                    consume(1);
                    return true;
                } else {
                    std::stringstream ss;

                    ss << "Expecting '}' to close group. Next char = '" <<  (int)peek(0)  << "'\n";

                    record_error(ss.str());
                    return false;
                }
            } else if(peek() == '(' ) {
                consume(1);
                skip();
                if (peek() == ')') {
                    consume(1);
                    parent->vtype = VT::LIST;
                    return true;
                } else {
                    record_error("Expecting ')' to close list.");
                    return false;
                }
            } else if(peek() == '[' ) {
                parent->vtype = VT::ARRAY;
                consume(1);

                if (! parse_typename(parent)) {
                    record_error("Invalid type specification for array");
                    return false;
                }

                skip();
                if (peek() == ']') {
                    consume(1);
                    return true;
                } else {
                    record_error("Expecting ']' to close array.");
                    return false;
                }
           

            } else {
                return parse_typename(parent);
            }            

        }

        //##############  keyspec ##########################

        bool parse_keyspec(SchemaNode* parent) {
            skip();
            ENTER;

            auto name = match_name(true);
            if ( ! name ) {
                return false;
            }

            bool required = false;

            if (match_char('!')) {
                required = true;
                consume(1);
            }

            skip();

            if (not match_char(':')) {
                record_error("Expecting : after key name");
                return false;
            }
            consume(1);

            auto new_key_spec = parent->add_subkey(*name);

            if (!new_key_spec) {
                record_error("key spec for "s + *name + " already defined in this context");
                return false;
            }

            new_key_spec->required = required;

            skip();

            return parse_constraint(new_key_spec);

        }

        //############## parse_ex_typespec #######################
        bool parse_ex_typespec(SchemaNode *parent){
            skip();
            ENTER;


            // check the longer alt first.
            if (match_keyword({"_type", "_t"})) {

                skip();

                if (! expect_char(':')) {
                    record_error("Expecting ':' between key and value");
                    RETURN(false);
                }

                if (!parse_typename(parent, true)) {
                    // parse_typename will have registered the error.
                    RETURN(false);
                }

                optional_sep();
                
                RETURN(true);
            }

            RETURN(false);

        }

        //############## parse_ex_required #######################
        bool parse_ex_required(SchemaNode *parent) {
            skip();
            ENTER;

            auto old_loc = current_loc;
            
            if (match_keyword({"_required", "_r"})) {

                skip();

                if (! expect_char(':')) {
                    record_error("Expecting ':' between key and value");
                    RETURN(false);
                }
                skip();
                auto opt_required = match_bool_value();
                if (!opt_required) {
                    record_error("Expecting a boolean literal (true/false)");
                    RETURN(false);
                }

                optional_sep();
                
                if (parent->required) {
                    record_error("Cannot use both the  '_required' tag and the '!' operator.",
                        old_loc);
                    RETURN(false);
                } else {
                    parent->required = *opt_required;
                }

                RETURN(true);
            }

            RETURN(false);

        }


        //############## parse_ex_arrtype #######################
        bool parse_ex_arrtype(SchemaNode *parent){
            skip();
            ENTER;


            // check the longer alt first.
            if (match_keyword({"_arrtype", "_at"})) {

                skip();

                if (! expect_char(':')) {
                    record_error("Expecting ':' between key and value");
                    RETURN(false);
                }

                if (!parse_typename(parent, true)) {
                    // parse_typename will have registered the error.
                    RETURN(false);
                }

                optional_sep();

                if (parent->vtype != VT::ARRAY) {
                    record_error("_arrtype tag is only valid if type is array");
                    RETURN(false);
                }
                
                RETURN(true);
            }

            RETURN(false);

        }

        //############## parse_ex_length #######################
        bool parse_ex_length(SchemaNode *parent){
            skip();
            ENTER;


            // check the longer alt first.
            if (match_keyword({"_length", "_len"})) {

                skip();

                if (! expect_char(':')) {
                    record_error("Expecting ':' between key and value");
                    RETURN(false);
                }

                auto range = parse_int_range(true);

                if (!range) {
                    RETURN(false);
                }

                parent->length = *range;

                optional_sep();

                if (parent->vtype != VT::ARRAY) {
                    record_error("_length tag is only valid if type is array");
                    RETURN(false);
                }
                
                RETURN(true);
            }

            RETURN(false);

        }

        //############## parse_ex_range #######################
        bool parse_ex_range(SchemaNode *parent, ValType vtype){
            skip();
            ENTER;

            if (match_keyword({"_range"})) {

                skip();

                if (! expect_char(':')) {
                    record_error("Expecting ':' between key and value");
                    RETURN_B(false);
                }

                bool type_ok = false;

                if (vtype == VT::INTEGER) {

                    auto range = parse_int_range(false);

                    if (!range) {
                        RETURN(false);
                    }
                    parent->int_range = *range;
                    type_ok = true;

                } else if (vtype == VT::FLOAT) {
                    auto range = parse_float_range();

                    if (!range) {
                        RETURN(false);
                    }
                    parent->float_range = *range;
                    type_ok = true;
                }

                optional_sep();

                if (not type_ok) {
                    record_error("_range tag is only valid if type is int or float");
                    RETURN_B(false);
                }

                
                RETURN_B(true);
            }

            RETURN_B(false);

        }
        //##############   parse_ex_default #####################
        bool parse_ex_default(SchemaNode * parent) {
            skip();
            ENTER;

            if (match_keyword({"_default", "_d"})) {

                skip();

                if (! expect_char(':')) {
                    record_error("Expecting ':' between key and value");
                    RETURN_B(false);
                }

                auto *setting = new Setting();
                if (! match_scalar_value(setting)) {
                    record_error("Invalid or missing value for default");
                    delete setting;
                    RETURN_B(false);
                }

                optional_sep();

                if (not parent->is_scalar()) {
                    record_error("Only scalar values allowed as defaults.");
                    delete setting;
                    RETURN_B(false);
                }

                parent->dflt.reset(setting);

                RETURN_B(true);

            }

            RETURN_B(false);

        }

        //##############   parse_extended_list #####################
        bool parse_extended_list(SchemaNode * parent) {
            skip();
            ENTER;

            bool at_least_one = false;
            if (parse_ex_typespec(parent)) {
                // sub tags check to make sure they
                // are in a valid context.
                parse_ex_required(parent);
                parse_ex_arrtype(parent);
                parse_ex_length(parent);
                auto p_vtype = parent->vtype == 
                        VT::ARRAY ? parent->array_type : parent->vtype;
                parse_ex_range(parent, p_vtype);
                parse_ex_default(parent);
            }

            if (parent->vtype == VT::GROUP) {
                at_least_one = true; 
                parse_keyspec_list(parent);
            } else if (parent->vtype == VT::ARRAY and 
                        parent->array_type == VT::GROUP) {
                at_least_one = parse_keyspec_list(parent);                
            } else if (parent->vtype == VT::NONE) {
                at_least_one = parse_keyspec_list(parent);
                if (at_least_one)
                    parent->vtype = VT::GROUP;
            } else {
                at_least_one = true;
            }

            if (parent->dflt) {
                if (parent->required) {
                    record_error("required keys cannot have defaults");
                    RETURN_B(false);
                }
                Validator v{errors};
                if (!v.validate(parent->dflt.get(), parent)) {
                    record_error("Default value does not match schema.");
                    RETURN_B(false);
                }
            }
            RETURN_B(at_least_one);
        }

        //##############   parse_keyspec_list #####################

        bool parse_keyspec_list(SchemaNode * parent) {
            skip();
            ENTER;

            bool at_least_one = false;
            while (1) {
                if (!parse_keyspec(parent)) break;
                at_least_one = true;

                optional_sep();

                skip();
            }

            RETURN_B(at_least_one);
        }


        //##############   do_parse  #####################
        
        bool do_parse() {
            ENTER;

            error_count = 0;

            //std::cout << "Parsing : " << src_text << "\n";

            skip();

            if (! parse_keyspec_list(schema)) {
                record_error("No keys seen");
                RETURN_B(false);
            }

            if (! eoi()) {
                std::stringstream ss;

                ss << "Not at end of input! Next char = '" <<  (int)peek(0)  << "'\n";
                record_error(ss.str());
                RETURN_B(false);
            }

            RETURN_B(not has_errors());
        }

    };
}