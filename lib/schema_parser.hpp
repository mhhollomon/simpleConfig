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



    struct SchemaParser : public ParserBase {

        SchemaNode *schema;

        SchemaParser(std::string_view src, SchemaNode* _schema, error_list & errlist) : 
            ParserBase(src, "Schema"s, errlist),
            schema{_schema}
        { }


        //##############  parse_type_constraint #######################

        bool parse_type_constraint(SchemaNode * keyspec) {

            ValType vtype = VT::NONE;

            if (match_string("int")) {
                vtype = VT::INTEGER;
            } else if (match_string("bool")) {
                vtype = VT::BOOL;
            } else if (match_string("float")) {
                vtype = VT::FLOAT;
            } else if (match_string("string")) {
                vtype = VT::STRING;
            } else if (match_string("any")) {
                vtype = VT::ANY;
            } else {
                record_error("Invalid type constraint for key specifier");
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
                new_key_spec->vtype = VT::GROUP;
                if (! parse_group(new_key_spec)) {
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


        //##############   parse_group #####################

        bool parse_group(SchemaNode * parent) {
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

            if (! parse_group(schema)) {
                return false;
            }

            if (! eoi()) {
                record_error("Not at end of input!");
                return false;
            }

            return has_errors();
        }


    };
}