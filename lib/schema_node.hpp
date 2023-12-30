#pragma once

#include "value_type.hpp"
#include "range.hpp"

#include <map>
#include <string>


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


        std::map<std::string, SchemaNode> subkeys;

        SchemaNode* add_subkey(const std::string &name) {
            auto [iter, done]  = subkeys.emplace(name, SchemaNode{name});

            if (done) {
                return &(iter->second);
            } else {
                return nullptr;
            }

        }

        bool contains_underbar_keys() {
            for (auto const &iter : subkeys) {
                if (iter.first.front() == '_')
                    return true;
            }
            return false;
        }

        bool contains_key(std::string k) {
             auto const & iter1 = subkeys.find(k); 
             return (iter1 != subkeys.end());   
        }

        bool is_extended() {
            return contains_key("_t") or 
                    contains_key("_type");
        }

    };

}