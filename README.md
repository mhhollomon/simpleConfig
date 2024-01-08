# simpleConfig
Simple configuration Library with schema validation.

For some configuration files, validating the structure of the parsed settings 
(required keys are present, no unknown keys are present, values have acceptable types.)
can be as time consuming to write code as the parsing of the syntax.

- Config language is based on [libconfig](https://github.com/hyperrealm/libconfig)
- C++17 based.
- Allows the user to specify a schema for the target configuration.
- Works nicely with FetchConfig.
- Error return and exception throwing interfaces available when interrogating
  the configuration.

## Differences from libconfig
- Arrays may have groups as elements.
  
- Only one integer type (long)
- Only one float type (double)
- Hex numbers are integers *only*

See [Config API](docs/API.md) for more details.

## Validation

Allows the validation of data types, data ranges, array sizes, required keys, 
and more.

Integers, floats, and strings can be constrained to be one of a small list (
  that is, like an enum )

See [Validation Schema](docs/VALIDATION_SCHEMA.md) docs for more information.

## Sample Usage

```C++
// sample usage
#include <simpleConfig.hpp>
#include <string>

std::string schema = R"DELIM(
// long schema ....
)DELIM"

simpleConfig::Config cfg;

if (!cfg.set_schema(schema)) {
  std::cerr << "Bad Schema\n";
  cfg.stream_errors(std::cerr);
}

if (! cfg.parse(file_name)) {
  std::cerr << "Yikes\n";
}

// Or
if (! cfg.parse(my_istream)) {
  std::cerr << "Yikes again!\n";
}
```

## TODO
- Add a way to set defaults for arrays in the schema.
- Add a way to set defaults for groups in the schema.
- Add parse locations to both settings and schema
  so that validation errors can reference them.
- Add code coverage metrics.