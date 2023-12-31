// Grammar is here : https://hyperrealm.github.io/libconfig/libconfig_manual.html#Configuration-File-Grammar
//
#pragma once

#include "value_type.hpp"
#include "parser_utils.hpp"
#include "setting.hpp"

#include <memory>
#include <sstream>
#include <fstream>
#include <functional>

#include <type_traits>

using namespace std::literals::string_literals;

namespace simpleConfig {
    
    // These make up the schema tree
    // that is consulted for the rules
    struct SchemaNode;


    struct Parser;
    struct SchemaParser;

    class Config {
        std::unique_ptr<Setting>cfg_;

        // can't use unique_ptr with incomplete types.
        Parser* parser_ = nullptr;
        SchemaParser* schema_parser_ = nullptr;
        SchemaNode* schema_tree_ = nullptr;

        error_list errors;

    public :

        bool parse_file(std::string file_name) {
            std::ifstream strm{file_name};
            return parse(strm);
        }
        bool parse(std::ifstream &strm) {
            // too many copies, but shouldn't matter much.
            std::stringstream buffer;
            buffer << strm.rdbuf();
            return parse(buffer.str());
        }

        bool set_schema(std::string schema_text);

        bool parse(const std::string &input) {
            return parse_with_schema(input);
        }

        Setting& get_settings() const {
            return *cfg_;
        }

        Setting& at_path(const std::vector<std::string> &v) {
            return get_settings().at_path(v);
        }

        const error_list &get_errors() const { return errors; }

        std::ostream &stream_errors(std::ostream& strm);

        ~Config();

    private:
        bool parse_with_schema(const std::string &input);

        bool check_against_schema();
    };


} // end namespace simpleConfig
