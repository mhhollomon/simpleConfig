#pragma once

#include "value_type.hpp"
#include "range.hpp"
#include "setting.hpp"

#include <map>
#include <string>
#include <memory>


namespace simpleConfig {

    struct SchemaNode {

        SchemaNode() = default;

        SchemaNode(const std::string &name) : name(name) 
        {}

        std::string name;
        bool required = false;
        ValType vtype = ValType::NONE;
        ValType array_type = ValType::NONE;

        Range<long> length{};
        Range<long> int_range{};
        Range<double> float_range{};
        std::unique_ptr<Setting> dflt;
        std::unique_ptr<Setting> enum_values;

        std::map<std::string, SchemaNode> subkeys;

        SchemaNode* add_subkey(const std::string &name) {
            auto [iter, done]  = subkeys.emplace(name, SchemaNode{name});

            if (done) {
                return &(iter->second);
            } else {
                return nullptr;
            }

        }

        bool is_scalar() const {
            return (
                vtype == ValType::INTEGER or
                vtype == ValType::BOOL or
                vtype == ValType::STRING or
                vtype == ValType::FLOAT);
        }

        bool contains_key(std::string k) {
             auto const & iter1 = subkeys.find(k); 
             return (iter1 != subkeys.end());   
        }


    };

}