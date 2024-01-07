#pragma once

#include "setting.hpp"
#include "schema_node.hpp"
#include "parser_utils.hpp"
#include "error_reporter.hpp"

#include <set>
#include <map>
#include <iostream>

namespace simpleConfig {

    struct Validator : public ErrorReporter {

        Validator(error_list &errlist) :
            ErrorReporter{"Validator"s, errlist}
        {}
    
        bool validate(
                Setting *setting_ptr, 
                const SchemaNode* schema_ptr ) {

            if (setting_ptr->is_group()) {
                return validate_group(setting_ptr, schema_ptr);
            } else if (setting_ptr->is_scalar()) {
                return validate_scalar(setting_ptr, schema_ptr);
            } else {
                return true;
            }
        }

        bool validate_scalar(
                Setting *setting_ptr, 
                const SchemaNode* schema_ptr ) {

            if (schema_ptr->vtype == ValType::ANY) {
                // No further real validations are available.
                return true;
            }

            if (setting_ptr->get_type() != schema_ptr->vtype) {
                record_error("setting and schema value type don't match", {});
                return false;
            }
            
            if (setting_ptr->is_integer()) {
                if (schema_ptr->int_range.limited) {
                    //std::cout << "  Checking key " << sett_name << " as range limited "<<
                    //    snode->int_range << "\n";
                    int v = setting_ptr->get<int>();
                    if (v > schema_ptr->int_range.max || v < schema_ptr->int_range.min ) {
                        record_error( "int value is out of range" , {});
                        return false;
                    }
                }

            }  else if (setting_ptr->is_float()) {
                if (schema_ptr->float_range.limited) {
                    //std::cout << "  Checking key " << sett_name << " as range limited "<<
                    //    snode->float_range << "\n";
                    int v = setting_ptr->get<double>();
                    if (v > schema_ptr->float_range.max || v < schema_ptr->float_range.min ) {
                        record_error( "float value is out of range" , {});
                        return false;
                    }
                }
            }

            return true;
        }

        bool validate_group(
                Setting * setting_ptr, 
                const SchemaNode* schema_ptr ) {
        
            // Keep track of the keys we've dealt with
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

                SchemaNode const *snode = nullptr;

                if (schema_match == schema_ptr->subkeys.end()) {
                    if (has_star) {
                        snode = &(star->second);
                        key_name_ok = true;
                        ftype = snode->vtype;
                        saw_star_key = true;
                    } else {
                        key_name_ok = false;
                    }
                } else {
                    snode = &(schema_match->second);
                    key_name_ok = true;
                    ftype = snode->vtype;
                }

                if (! key_name_ok) {
                    record_error("Key = "s + sett_name + " is not allowed.", {});
                }
                
                if (ftype != ValType::ANY && setting.get_type() != ftype )
                    record_error("Key = "s + sett_name + " has wrong type.", {});
                
                seen.insert(sett_name);

                if (snode) {
                    if (setting.is_group()) {
                        validate(&setting, snode);

                    } else if (setting.is_array()) {
                        if (setting.array_type() != 
                            snode->array_type) {

                            record_error("Key = "s + sett_name + 
                                " has wrong type for array elements.", {});

                        }

                        auto range = snode->length;
                        auto length = setting.count();
                        if (range.max > 0 && (length < range.min || length > range.max)) {
                            record_error("Number of array elements is out of range", {});
                        }

                        if (snode->array_type == ValType::INTEGER && snode->int_range.limited) {
                            for (auto const &child : setting) {
                                int v = child.get<int>();
                                if (v > snode->int_range.max || v < snode->int_range.min ) {
                                    record_error("Array integer element value is out of range" , {});
                                }
                            }
                        }   else if (snode->array_type == ValType::FLOAT && snode->float_range.limited) {
                            for (auto const &child : setting) {
                                int v = child.get<double>();
                                if (v > snode->float_range.max || v < snode->float_range.min ) {
                                    record_error("Array float element value is out of range" , {});
                                }
                            }
                        } else if (snode->array_type == ValType::GROUP) {
                            for (auto &child : setting) {
                                validate(&child, snode);
                                
                            }
                        }


                    } else if (setting.is_scalar()) {
                        validate_scalar(&setting, snode);
                    }        
                }
            }

            // If needed, check that we saw a key for the required '*' entry

            if (star_required && not saw_star_key)
                record_error("Key = "s + star->first + 
                    " requires a 'star' entry", {});

            // Run through the schema checking that required keys
            // were seen.

            for (const auto &[name, snode] : schema_ptr->subkeys) {
                if (name == "*") {
                    continue;
                }
                const auto &seen_iter = seen.find(name);
                if (seen_iter == seen.end()) {
                    if (snode.required) {
                        record_error("Required key = "s + name + 
                            " is not present.", {});

                    } else if (snode.dflt) {
                        setting_ptr->add_child(name, *snode.dflt);
                    }
                }
            }

            return not has_errors();

        }


    };
}