# simpleConfig
Simple configuration Library with schema validation.

For some configuration files, validating the structure of the parsed settings 
(required keys are present, no unknown keys are present, values have acceptable types.)
can be as time consuming to write code as the parsing of the syntax.

## Design Goals
- Based on [libconfig](https://github.com/hyperrealm/libconfig)
- C++17 based.
- will allow the user to specify a schema for the target configuration.
- will be setup to work nicely with FetchConfig.
- throw as little as possible without making the interface horrible.


## Differences from libconfig
- Only one integer type (long)
- Only one float type (double)
- Hex numbers are integers *only*

See [Config API](docs/API.md) for more details.

## Validation

Allows the validation of data types, data ranges, array sizes, required keys, 
and more.

See [Validation Schema](docs/VALIDATION_SCHEMA.md) docs for more information.

## Sample Usage

```C++
// sample usage
#include <simpleConfig.hpp>
#include <string>

std::string schema = R"DELIM(
// long schema ....
)DELIM"s

simpleConfig::Config cfg;

if (!cfg.set_schema(schema)) {
  std::cerr << "Bad Schema\n";
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
- Add a way to set defaults in the schema
- Add a way to range constrain floats
- Add a way to have an "enum" like contraint
- Add parse locations to both settings and schema
  so that validation errors can reference them.