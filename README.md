# simpleConfig
Simple configuration Library with schema validation.

Patterned after [libConfig](https://github.com/hyperrealm/libconfig)

```
    foo : 123
    bar : "hello";
    crash-and-burn = True,
    subgroup = {
        a : 3.124;
        b : "some string" "in two parts"
    }

    a_key : [ 1, 2, 3 ];
    a2_key [ "x", "y", "z" "extend" ]
```

The following are keywords in the validation schema and may not be used as
keys in a configuration file.

- str
- int
- bool
- float
- list
- any

key/value pairs are delimited by blanks, commas, or semi-colons.
```
#This is a valid configuration
foo:123 bar:"hello"
```

```
Three kinds of comments
# Scripting style
// C++ line comments
/* C block comments */
```

## Validation Schema


Schemas have a different but related format.

Valid types are int, string, float, bool, any

The _any_ type allows composites (groups, lists, arrays)

Note that - unlike the config file - only a colon is allowed between the
key and type specifier.

```

# foo must be present and an integer.
foo! : int
# bar may be present but must be a string if present.
bar : string

# other keys may be present, but must be integers
* : int

# if the bang is used with '*', then at least one
# key must be present that needs the star to
# be allowed.
baz : bool
*! : any

# This would not match (baz is explicitly allowed)
baz : true

# This would match (foo is allowed by the '*', baz is not required)
foo : 42

# nested groups can also be validated.
baz! : {
    my_sub_key : int,
    * : any
}

# Array may be specified.
# Note that 'any' is still valid
# inside array. What it means is that
# the entries in the array may be any type
# but they still must be all the same type.
#
# You cannot currently constrain the length of the array.
array_key : [ int ]

# Array may on have scalars (flot, int, bool) as elements.

# List may be specified. There is no validation of entries
# of the list

list_key = ();

```