#pragma once

#include "setting.hpp"
#include "schema_node.hpp"
#include "parser_utils.hpp"

#include <set>
#include <map>

namespace simpleConfig {

    struct Validator : public ErrorReporter {

        Validator(error_list &errlist) :
            ErrorReporter{"Validator"s, errlist}
        {}
    
        bool validate(
                Setting * setting_ptr, 
                const SchemaNode* schema_ptr ) {

            std::set<std::string> seen;

            auto star = schema_ptr->subkeys.find("*");
            bool has_star = (star != schema_ptr->subkeys.end());
            bool saw_star_key = false;
            bool star_required = false;
            if (has_star) {
                star_required = star->second.required;
            }



            // Run through the config making sure the keys that are there
            // are supposed to be there and of the correct type
            for (const auto& setting_iter : setting_ptr->enumerate()) {
                const auto & schema_match = schema_ptr->subkeys.find(setting_iter.first);

                //std::cout << "Check key - " << setting_iter.first << "\n";

                bool key_name_ok = false;
                ValType ftype = ValType::NONE;
                if (schema_match == schema_ptr->subkeys.end()) {
                    if (has_star) {
                        key_name_ok = true;
                        ftype = star->second.vtype;
                        saw_star_key = true;
                    } else {
                        key_name_ok = false;
                    }
                } else {
                    key_name_ok = true;
                    ftype = schema_match->second.vtype;
                }

                if (! key_name_ok)
                    record_error("Key = "s + setting_iter.first + " is not allowed.", {});
                
                if (ftype != ValType::ANY && setting_iter.second.get_type() != ftype )
                    record_error("Key = "s + setting_iter.first + " has wrong type.", {});
                
                seen.insert(setting_iter.first);

                if (setting_iter.second.is_group()) {
                    validate(&setting_iter.second, &(schema_match->second));
                }

                if (setting_iter.second.is_array()) {
                    if (setting_iter.second.array_type() != 
                        schema_match->second.array_type) {

                        record_error("Key = "s + setting_iter.first + 
                            " has wrong type for array elements.", {});

                    }

                }           
            }

            // If needed, check that we saw a key for the required '*' entry

            if (star_required && not saw_star_key)
                record_error("Key = "s + star->first + 
                    " requires a 'star' entry", {});

            // Run through the schema checking that required keys
            // were seen.

            for (const auto &c : schema_ptr->subkeys) {

                if ( ! c.second.required)
                    continue;
                
                const auto &seen_iter = seen.find(c.first);
                if (seen_iter == seen.end()) {
                    record_error("Required key = "s + c.first + 
                        " is not present.", {});
                }
            }

            return has_errors();

        }


        };
}