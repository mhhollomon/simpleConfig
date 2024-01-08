#include "value_type.hpp"


namespace simpleConfig {
    bool valtype_is_scalar(ValType v) {
        return (v == ValType::STRING
            or v == ValType::BOOL
            or v == ValType::INTEGER
            or v == ValType::FLOAT);
    }

}
