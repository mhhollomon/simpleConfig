
#include <validator.hpp>

namespace simpleConfig {

    bool Validator::validate_scalar(
            Setting *setting_ptr, 
            const SchemaNode* schema_ptr,
            ValType stype ) {
        
        if (stype == ValType::NONE) {
            stype = schema_ptr->vtype;
        }

        if (stype == ValType::ANY) {
            // No further real validations are available.
            return true;
        }

        if (setting_ptr->get_type() != stype) {
            record_error("setting and schema value type don't match", {});
            return false;
        }
        
        if (stype == ValType::INTEGER) {
            auto v = setting_ptr->get<int>();
            if (schema_ptr->int_range.limited) {
                //std::cout << "  Checking key " << sett_name << " as range limited "<<
                //    snode->int_range << "\n";
                if (v > schema_ptr->int_range.max || v < schema_ptr->int_range.min ) {
                    record_error( "int value is out of range" , {});
                    return false;
                }
            } else if (schema_ptr->enum_values) {
                bool found = false;
                for (auto &e : *(schema_ptr->enum_values)) {
                    if (e.get<int>() == v) {
                        found = true;
                        break;
                    }
                }
                if (not found) {
                    record_error("Int value is not in enum list", {});
                    return false;
                }
            }

        }  else if (stype == ValType::FLOAT) {
            auto v = setting_ptr->get<double>();
            if (schema_ptr->float_range.limited) {
                //std::cout << "  Checking key " << sett_name << " as range limited "<<
                //    snode->float_range << "\n";
                if (v > schema_ptr->float_range.max || v < schema_ptr->float_range.min ) {
                    record_error( "float value is out of range" , {});
                    return false;
                }
            } else if (schema_ptr->enum_values) {
                bool found = false;
                for (auto &e : *(schema_ptr->enum_values)) {
                    if (e.get<double>() == v) {
                        found = true;
                        break;
                    }
                }
                if (not found) {
                    record_error("Float value is not in enum list", {});
                    return false;
                }

            }
        }  else if (stype == ValType::STRING) {
            auto v = setting_ptr->get<std::string>();
            if (schema_ptr->enum_values) {
                bool found = false;
                //std::cout << "   Checking that '" << v << "' is in enum list\n";
                for (auto &e : *(schema_ptr->enum_values)) {
                    auto enum_value = e.get<std::string>();
                    //std::cout << "   - checking against '" << enum_value << "'\n";
                    if (e.get<std::string>() == v) {
                        //std::cout << "   - - YEA!\n";
                        found = true;
                        break;
                    }
                }
                if (not found) {
                    record_error("String value is not in enum list", {});
                    return false;
                }

            }

        }

        return true;
    }


    bool Validator::validate_array(
            Setting * setting_ptr, 
            const SchemaNode* schema_ptr ) {

        bool okay = true;

        if (setting_ptr->array_type() != 
            schema_ptr->array_type) {

            record_error("Key has wrong type for array elements.", {});
                return false;

        }

        auto range = schema_ptr->length;
        auto length = setting_ptr->count();
        if (range.max > 0 && (length < range.min || length > range.max)) {
            record_error("Number of array elements is out of range", {});
            okay = false;

        }

        if (valtype_is_scalar(schema_ptr->array_type)) {
            for (auto &child : *setting_ptr) {
                okay = okay && validate_scalar(&child, schema_ptr, schema_ptr->array_type);
            }
        } else if (schema_ptr->array_type == ValType::GROUP) {
            for (auto &child : *setting_ptr) {
                okay = okay && validate_group(&child, schema_ptr);
                
            }
        }

        return okay;
    }


    bool Validator::validate_group(
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
                    validate_group(&setting, snode);

                } else if (setting.is_array()) {
                    validate_array(&setting, snode);

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


}