
#include <schema_parser.hpp>
#include <config_parser.hpp>
#include "validator.hpp"

#include <simpleConfig.hpp>

#include <set>

#include <iostream>

namespace simpleConfig {

    bool Config::set_schema(std::string schema) {
        if (schema_tree_) delete schema_tree_;
        schema_tree_ = new SchemaNode();
        if (schema_parser_) delete schema_parser_;
        schema_parser_ = new SchemaParser(schema, schema_tree_, errors);

        auto parse_ok = schema_parser_->do_parse();

        if (not parse_ok) {
            delete schema_tree_;
            schema_tree_ = nullptr;
        }

        return parse_ok;
    }



    bool Config::parse_with_schema(const std::string &input){

        cfg_.reset(new Setting(VT::GROUP));

        std::cout << "Parsing : " << input << "\n";

        if (parser_) delete parser_;
        parser_ = new Parser(input, cfg_.get(), errors);

        auto parse_ok = parser_->do_parse();

        if (! parse_ok) {
            std::cout << "--- CONFIG PARSE FAILED ---\n";
            stream_errors(std::cout);
        }

        std::cout << "schema_tree = " << schema_tree_ << "\n";

        if (parse_ok && schema_tree_) {
            std::cout << "-- Okay to validate\n";
            return check_against_schema();
        }

        return parse_ok;
    }



    bool Config::check_against_schema() {
        // double check;
        //if (! cfg_ || ! schema_tree_ ) return true;

        auto validator = Validator(errors);

        return validator.validate(cfg_.get(), schema_tree_ );

    }

    std::ostream &Config::stream_errors(std::ostream &strm) {
        strm << parser_->errors;

        return strm;
    }

    Config::~Config() {
        if (parser_) {
            delete parser_;
            parser_ = nullptr;
        }

        if (schema_tree_) {
            delete schema_tree_;
            schema_tree_ = nullptr;
        }

        if (schema_parser_) {
            delete schema_parser_;
            schema_parser_ = nullptr;
        }
    }

} // end namespace simpleConfig
