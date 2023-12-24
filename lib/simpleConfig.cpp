
#include <schema_parser.hpp>
#include <config_parser.hpp>

#include <simpleConfig.hpp>

#include <set>

#include <iostream>

namespace simpleConfig {

    bool Config::set_schema(std::string schema) {
        if (schema_tree_) delete schema_tree_;
        schema_tree_ = new SchemaNode();
        if (schema_parser_) delete schema_parser_;
        schema_parser_ = new SchemaParser(schema, schema_tree_);

        return schema_parser_->do_parse();
    }



    bool Config::parse_with_schema(const std::string &input){

        cfg_.reset(new Setting(VT::GROUP));

        std::cout << "Parsing : " << input << "\n";

        if (parser_) delete parser_;
        parser_ = new Parser(input, cfg_.get());

        auto parse_ok = parser_->do_parse();

        if (! parse_ok) {
            stream_errors(std::cout);
        }

        std::cout << "schema_tree = " << schema_tree_ << "\n";

        if (parse_ok && schema_tree_) {
            std::cout << "-- Okay to validate\n";
            return check_against_schema();
        }

        return parse_ok;
    }


    bool schema_validate(
            Setting * setting_ptr, 
            const SchemaNode* schema_ptr ) {

        std::set<std::string> seen;

        auto star = schema_ptr->subkeys.find("*");
        bool has_star = (star != schema_ptr->subkeys.end());


        // Run through the config making sure the keys that are there
        // are supposed to be there and of the correct type
        for (const auto& setting_iter : setting_ptr->enumerate()) {
            const auto & schema_match = schema_ptr->subkeys.find(setting_iter.first);

            std::cout << "Check key - " << setting_iter.first << "\n";

            bool key_name_ok = false;
            ValType ftype = ValType::NONE;
            if (schema_match == schema_ptr->subkeys.end()) {
                if (has_star) {
                    key_name_ok = true;
                    ftype = star->second.vtype;
                } else {
                    key_name_ok = false;
                }
            } else {
                key_name_ok = true;
                ftype = schema_match->second.vtype;
            }

            if (! key_name_ok)
                throw std::runtime_error("Key = "s + setting_iter.first + " is not allowed.");
            
            if (ftype == VT::ANY || setting_iter.second.get_type() != ftype )
                throw std::runtime_error("Key = "s + setting_iter.first + " has wrong type.");
            
            seen.insert(setting_iter.first);

            if (setting_iter.second.is_group()) {
                schema_validate(&setting_iter.second, &(schema_match->second));
            }           
        }

        // Run through the schema checking that required keys
        // were seen.

        for (const auto &c : schema_ptr->subkeys) {

            if ( ! c.second.required)
                continue;
            
            const auto &seen_iter = seen.find(c.first);
            if (seen_iter == seen.end()) {
                throw std::runtime_error("Required key = "s + c.first + " is not present.");
            }
        }

        return true;

    }



    bool Config::check_against_schema() {
        // double check;
        //if (! cfg_ || ! schema_tree_ ) return true;

        return schema_validate(cfg_.get(), schema_tree_ );
    }

    std::ostream &Config::stream_errors(std::ostream &strm) {
        strm << parser_->errors;

        return strm;
    }

    Config::~Config() {
        if (parser_) delete parser_;
        if (schema_tree_) delete schema_tree_;
        if (schema_parser_) delete schema_parser_;
    }

} // end namespace simpleConfig
