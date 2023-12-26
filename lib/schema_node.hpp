#pragma once

#include <map>
#include <string>

#include "value_type.hpp"

namespace simpleConfig {

    struct SchemaNode {

        SchemaNode() = default;

        SchemaNode(const std::string &name) : name(name) 
        {}

        std::string name;
        bool required = false;
        ValType vtype = ValType::NONE;
        ValType array_type = ValType::NONE;

        std::map<std::string, SchemaNode> subkeys;

        SchemaNode* add_subkey(const std::string &name) {
            auto [iter, done]  = subkeys.emplace(name, SchemaNode{name});

            if (done) {
                return &(iter->second);
            } else {
                return nullptr;
            }

        }

    };

}