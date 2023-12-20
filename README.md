# simpleConfig
Simple configuration Library with schema validation

```
{
    foo : 123
    bar : "hello"
    crash-and-burn : True
}
```

The following are keywords in the validation schema and may not be used as
keys in a configuration file.

- str
- int
- bool

```
config-file ::= '{' ( <key> ':' <value> )+ '}'

key ::= [a-zA-Z][a-zA-Z0-9-_]*

value ::= scalar_value

scalar_value ::= integer | string | boolean

integer ::= [-+]?[0-9]+ | 0[xX][0-9a-fA-F]
string ::= #double quote delimited string
boolean ::= [Tt][Rr][Uu][Ee] | [Ff][Aa][Ll][Ss][Ee]
```

key/value pairs are delimited by blanks.
```
#This is valid
{foo:123 bar:"hello"}
```

```
Three kinds of comments
# Scripting style
// C++ line comments
/* C block comments */
```

## Validation Schema

```
# Yes, this means that a schema file is not
# a valid config file.
{
    # foo must be present and an integer
    foo! : int
    # bar may be present but must be a string if so
    bar? : str
    # other keys may be present, but must be integers
    * : int
}
```