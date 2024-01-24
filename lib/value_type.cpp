#include "value_type.hpp"


namespace simpleConfig {
    bool valtype_is_scalar(ValType v) {
        return (v == ValType::STRING
            or v == ValType::BOOL
            or v == ValType::INTEGER
            or v == ValType::FLOAT);
    }

    bool valtype_is_composite(ValType v) {
        return (v == ValType::ARRAY
            or v == ValType::GROUP
            or v == ValType::LIST
            );
    }
}
