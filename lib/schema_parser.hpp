#pragma once

#include "parser_base.hpp"

#include "value_type.hpp"

#include "schema_node.hpp"

#include <string>
#include <string_view>
#include <charconv>
#include <optional>
#include <sstream>
#include <map>
#include <memory>

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


        //##############  parse_type_constraint #######################

        bool parse_type_constraint(SchemaNode * keyspec, bool extended = false) {

            ValType vtype = VT::NONE;

            for (auto const &iter : type_map ){
                if (not extended and not iter.second.normal) continue;
                if (match_string(iter.first)) {
                    vtype = iter.second.vtype;
                    break;
                }
            }

            if (vtype == VT::NONE) {
                record_error("Invalid or missing type constraint for key specifier");
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


        //##############  key spec ##########################

        bool parse_keyspec(SchemaNode* parent) {

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

            if (peek() == '{') {
                consume(1);
                skip();
                if (! parse_extended_group(new_key_spec)) {
                    return false;
                }
                skip();
                if (peek() == '}') {
                    consume(1);
                    return true;
                } else {
                    record_error("Expecting '}' to close group.");
                    return false;
                }
            } else if(peek() == '(' ) {
                consume(1);
                skip();
                if (peek() == ')') {
                    consume(1);
                    new_key_spec->vtype = VT::LIST;
                    return true;
                } else {
                    record_error("Expecting ')' to close list.");
                    return false;
                }
            } else if(peek() == '[' ) {
                new_key_spec->vtype = VT::ARRAY;
                consume(1);
                skip();

                if (! parse_type_constraint(new_key_spec)) {
                    record_error("Invalid type specification ffor array");
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
                return parse_type_constraint(new_key_spec);
            }            
        }

        //############## parse_type_spec #######################
        bool parse_type_spec(SchemaNode *parent){
            if (match_string("_t") or match_string("_type")) {
                skip();
                if (peek() != ':') {
                    record_error("Expecting ':' between key and value");
                    return false;
                }
                skip();

                if (!parse_type_constraint(parent, true))
                    // parse_type_constraint will have registered the error.
                    return false;

                skip();
                if (match_chars(0, ";,"))
                    consume(1);
                
                return true;
            }

            return false;

        }

        //##############   parse_extended_group #####################
        bool parse_extended_group(SchemaNode * parent) {

            bool at_least_one = false;
            parse_type_spec(parent);

            if (parent->vtype == VT::GROUP) {
                at_least_one = true; 
                parse_simple_group(parent);
            } else if (parent->vtype == VT::NONE) {
                at_least_one = parse_simple_group(parent);
            }

            return at_least_one;
        }
        //##############   parse_simple_group #####################

        bool parse_simple_group(SchemaNode * parent) {
            bool at_least_one = false;
            while (1) {
                if (!parse_keyspec(parent)) break;
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

            if (! parse_simple_group(schema)) {
                record_error("No keys seen");
                return false;
            }

            if (! eoi()) {
                record_error("Not at end of input!");
                return false;
            }

            return not has_errors();
        }


    };
}