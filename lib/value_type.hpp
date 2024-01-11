#pragma once

namespace simpleConfig {

    enum class ValType { 
        NONE, STRING, BOOL, INTEGER, 
        FLOAT, GROUP, LIST, ARRAY, ANY 
    };

    bool valtype_is_scalar(ValType v);
    bool valtype_is_composite(ValType v);
    
}