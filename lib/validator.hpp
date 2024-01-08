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
            } else if (setting_ptr->is_array()) {
                return validate_array(setting_ptr, schema_ptr);
            } else if (setting_ptr->is_scalar()) {
                return validate_scalar(setting_ptr, schema_ptr);
            } else {
                // This would be a list or ANY. No validations.
                return true;
            }
        }

        bool validate_scalar(
            Setting *setting_ptr, 
            const SchemaNode* schema_ptr,
            ValType stype = ValType::NONE );


        bool validate_array(
                Setting * setting_ptr, 
                const SchemaNode* schema_ptr );


        bool validate_group(
                Setting * setting_ptr, 
                const SchemaNode* schema_ptr );

    };
}