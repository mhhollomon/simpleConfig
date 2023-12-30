# Validation Schema Language

## Validation Schema


Valid types are int, string, float, bool, any

The _any_ type allows composites (groups, lists, arrays)

Note that - unlike the config file - only a colon is allowed between the
key and type specifier.

There are two formats, a concise format and expanded format. They can
be mixed in a single schema.

### Concise format

This is terse - on purpose. However, it lacks features that the expanded
format allow.

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
# You cannot constrain the length of the array in the
# concise format (see the extended format).
array_key : [ int ]

# Arrays may only have scalars (float, int, bool, string) as elements.

# List may be specified. There is no validation of entries
# of the list

list_key = ();
```

### extended format

Allows for more features, but is more wordy.

```
# _type (_t) spec must come first - always.
# if the _type tag is not seen, the parser
# assumes this is a key that has a subgroup under it.
#
key : { _type : string, _required : true }

key2 : {
    # subgroup with two required keys
    x! = int,
    y = { _t : int, _r : true }
}

key3 {
    #parse error : '_' keys are not valid except if _type is seen.
    _required : true,
    spats : int
}

# 'any' can be used as the type. Only the _required
# tag will be valid for such an entry.

# 'int' and 'float' keys can be constrained as to value range.
# The value must be -100 >= v =< 100
key : { _t : int; _range : [-100, 100] }
# Note that the range numbers must be floats (have the decimal point in them).
flt_key : { _t:float; _range:[-100.0, 42.0]}

# Arrays can be constrained for length as well as value.
# The array can be 3, 4, or 5 elements long.
# Each element must be -100 >= e =< 30
key : { _t : array, _arrtype : int _length : [3 , 5], _range : [-100, 30]}
flt_key : { _t : array, _at : float _len : [3 , 5], _range : [-100.0, 30.666]}

# A single number in the length means the array must be exactly that long.
key : { _t : array, _at : float; _len : [2] }


```
## Current limitations
- Only arrays may be length restricted (not lists).

## EBNF

Main point is tha the `ex_*` tags must show up in a specific order
(though any irrelevant ones can be missing. )


Things between slashes `/../` are to be read as regular expressions.

```bnf
Spec = keyspec_list eoi

eoi = # end of input marker.

keyspec_list = ( keyvalue sep? )+

keyspec = name '!'? ':' constraint

sep = /[;,]/

name = /\* | [a-zA-z][a-zA-Z0-9_-]*/

constraint = typename | '{' extended_spec '}'
    | '[' typename ']' | '(' ')'

typename = 'int' | 'float' | 'bool' | 'string' | 'any'

# Note this implies that the ex_* tags MUST show up in
# the order specified.
extended_list = 
    ex_typespec ex_required? 
        ex_arrtype? ex_length? 
        ex_range? keyspec_list? |
    keyspec_list
     

ex_typespec = ( '_t' | '_type' ) ':' extended_typename sep?
ex_required = ( '_r' | '_required' ) ':' bool_value sep?
ex_arrtype  = ( '_at' | "_arrtype' ) ':' int_value sep?
ex_length   = ( '_len' | '_length' ) ':' range_value sep?
ex_range   =  '_range' ':' range_value sep?

extended_typename = typename | 'group' | 'list' | 'array'
range_value = '[' int_value sep? (int_value sep?)? ']' sep?

bool_value = /[Tt][Rr][Uu][Ee] | [Ff][Aa][Ll][Ss][Ee]/
int_value = /[+-]?[0-9]+ | /0[xX][0-9]+/
```