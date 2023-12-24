#pragma once

#include "parser_base.hpp"

#include "value_type.hpp"

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

    struct SchemaNode {

        SchemaNode() = default;

        SchemaNode(const std::string &name) : name(name) 
        {}

        std::string name;
        bool required = false;
        ValType vtype = ValType::NONE;
        std::map<std::string, SchemaNode> subkeys;

        SchemaNode* add_spec(const std::string &name) {
            auto [iter, done]  = subkeys.emplace(name, SchemaNode{name});

            if (done) {
                return &(iter->second);
            } else {
                return nullptr;
            }

        }

    };

    struct SchemaParser : public ParserBase {

        SchemaNode *schema;

        SchemaParser(std::string_view src, SchemaNode* _schema) : 
            ParserBase(src), schema{_schema}
        { }

        //##############   match_name #######################

        std::optional<std::string> match_name() {

            int pos = 0;
            if (peek(pos) == '*') {
                consume(1);
                return std::string("*");

            } else if (std::isalpha(peek(pos))) {
                pos += 1;
            } else {
                return std::nullopt;
            }

            while (valid_pos(pos)) {
                if ( peek(pos) == '_' or
                        std::isalnum(peek(pos))) {
                    pos += 1;
                } else break;
            }


            auto retval = current_loc.sv.substr(0, pos);
            consume(pos);

            return std::string(retval);
        }

        //##############   match_name #######################

        bool parse_type_constraint(SchemaNode * keyspec) {

            if (match_string("int")) {
                keyspec->vtype = VT::INTEGER;
            } else if (match_string("bool")) {
                keyspec->vtype = VT::BOOL;
            } else if (match_string("float")) {
                keyspec->vtype = VT::FLOAT;
            } else if (match_string("string")) {
                keyspec->vtype = VT::STRING;
            } else if (match_string("any")) {
                keyspec->vtype = VT::ANY;
            } else {
                record_error("Invalid type constraint for key specifier");
                return false;
            }

            if (! check_string_end()) {
                record_error("Invalid type constraint for key specifier");
                return false;
            }

            return true;
        }


        //##############  key spec ##########################

        bool parse_keyspec(SchemaNode* parent) {

            auto name = match_name();
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

            auto new_key_spec = parent->add_spec(*name);

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
                parse_group(new_key_spec);
                skip();
                if (peek() == '}') {
                    consume(1);
                    return true;
                } else {
                    record_error("Expecting '}' to close group.");
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

            skip();

            parse_group(schema);

            if (! eoi()) {
                record_error("Not at end of input!");
                return false;
            }

            return errors.empty();
        }


    };
}