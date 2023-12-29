#pragma once

#include "setting.hpp"
#include "schema_node.hpp"
#include "parser_utils.hpp"
#include "error_reporter.hpp"

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
            for (const auto& [sett_name, setting] : setting_ptr->enumerate()) {
                const auto & schema_match = schema_ptr->subkeys.find(sett_name);

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
                    record_error("Key = "s + sett_name + " is not allowed.", {});
                
                if (ftype != ValType::ANY && setting.get_type() != ftype )
                    record_error("Key = "s + sett_name + " has wrong type.", {});
                
                seen.insert(sett_name);

                auto const &sn = schema_match->second;

                if (setting.is_group()) {
                    validate(&setting, &sn);
                } else if (setting.is_array()) {
                    if (setting.array_type() != 
                        sn.array_type) {

                        record_error("Key = "s + sett_name + 
                            " has wrong type for array elements.", {});

                    }

                    auto range = sn.length;
                    auto length = setting.count();
                    if (range.max > 0 && (length < range.min || length > range.max)) {
                        record_error("Number of array elements is out of range", {});
                    }

                    if (sn.array_type == VT::INTEGER && sn.range_limited) {
                        int index = -1;
                        for (auto const &child : setting.enumerate()) {
                            index += 1;
                            int v = child.second.get<int>();
                            if (v > sn.range.max || v < sn.range.min ) {
                                record_error("Array element value is out of range" , {});
                            }
                        }
                    }

                } else if (setting.is_integer()) {
                    if (sn.range_limited) {
                        int v = setting.get<int>();
                        if (v > sn.range.max || v < sn.range.min ) {
                            record_error( "scalar value is out of range" , {});
                        }
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

            return not has_errors();

        }


        };
}