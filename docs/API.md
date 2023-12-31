# API

## simple configuration file example

key/value pairs are delimited by blanks, commas, or semi-colons.

```
    foo : 123,
    bar : "hello";
    crash-and-burn = True,
    subgroup = {
        a : 3.124;
        b : "some string" "in two parts"
    }

    a_key : [ 1, 2, 3 ];
    a2_key = [ "x", "y", "z" "extend" ]

    a_list = ( 1, "string", true);
```

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

See the [libconfig](https://hyperrealm.github.io/libconfig/) docs for more
detail.



## class Config

- `bool parse_file(std::string file_name)`
- `bool parse(std::ifstream& strm)`
- `bool parse(std::string& input)`

Prse the given information and store a setting tree for it. The return tells
you if it succeeded or not. If the return value is `false`, the errors are most
likely available with `stream_errors`.

- `Setting& get_settngs()`

Return a reference to the setting tree. If the last parse failed, this will be
partial, but valid until the point of failure.

- `std::ostream& stream_errors(std::ostream& strm)`

Format the error message gathered during parsing and place on the output
stream. 

## class Setting

The heart of the system. A Setting represents a value (not a key/value) - it
can be either a scalar (long, bool, double, string) or composite (group, list,
array).


### Constructors
-  `Setting(setting_type t = setting_type::BOOL)`

Create a Setting of the desired type of value. The actual
value is the default initialized value for the type.

- `Setting(bool b)`
- `Setting(int i)`
- `Setting(long i)`
- `Setting(double f)`
- `Setting(std::string s)`
- `Setting(const char * c)`

Create a Setting with the desired value and type. Some apparent duplicates are
there to help disabiguate the overloads.

### Type probes

- `bool is_composite()`
- `bool is_scalar()`

- `bool is_boolean()`
- `bool is_integer()`
- `bool is_float()`
- `bool is_string()`
- `bool is_group()`
- `bool is_list()`
- `bool is_array()`

- `bool is_numeric()`

### Setting/Updating the Scalar Value

- `Setting & set_value(bool v)`
- `Setting & set_value(int v)`
- `Setting & set_value(long v)`
- `Setting & set_value(double v)`
- `Setting & set_value(const std::string v)`
- `Setting & set_value(const char * v)`

Update the Setting to have the value and type specified. These are mutators.
The reference to the object is returned to facilitate chaining.

If the Setting is currently composite, these will remove any attached children
and switch the type to the correct scalar type.

**NOTE** The reference may become invalid if more children are added to the
composite. Don't hold on to it for long.


### Working with composites

- `Setting& make_list()`
- `Setting& make_group()`
- `Setting& make_array()`

Switches the Setting to be the requested type of composite. If it already is
correct, this is a no-op. If the type actually changes, any existing children
will be discarded.

- `int count()`

Return how many children the composite has. Actually also works with scalars
where it always returns 0.

- `Setting& add_child(setting_type t)`

Add a child of type `t`. The value for scalars will be the default initialized
value. The throws an exception for groups since children for groups must have
names. Also throws for scalars.

**NOTE** The reference may become invalid if more children are added to the
composite. Don't hold on to it for long.

- `template<typename T> Setting& add_child(T value)`

Attempts to deduce the correct scalar type for T and create a child of that
type initialized to the value. This throws an exception for groups since
children for groups must have names. Also throws for scalars.

**NOTE** The reference may become invalid if more children are added to the
composite. Don't hold on to it for long.

- `Setting& add_child(std::string name, setting_type t)`
- `Setting& add_child(std::string name, T value)`

Variants of the above for groups. Throws for scalars or non-group composites.

**NOTE** The reference may become invalid if more children are added to the
composite. Don't hold on to it for long.

- `Setting &at(int idx)`

Return a reference to the child at index idx. Negative values count from the
end. The method throws if the index is out of range or the Setting is a scalar.

Note that this does work for groups, but you don't get access to the name.

- `bool exists(std::string name)`

Checks if a given key exists in a group. Returns true if:
- The Setting is a group
- The key exists.
Returns true otherwise.

#### Setting& at(std::string name)

Returns a reference to the child added with name `name`. Throws if such a child
does not exist or the Setting is not a group.

- `iterator& begin()`
- `iterator& end()`

Return the STL style iterators. These work for all composite types. However,
for groups, it *ONLY* returns the values and it returns them in *insert* order
rather than key order.

Range-for is supported:

```c++
Setting & setting{Setting::setting_type::LIST};

setting.add_child(1);
// ...

for (auto &c : setting) {
    /* something */
}
```

- `enumerator & enumerate()`

This is for use with groups and will throw if not a group. This returns the
reference to an object which can iterate over the groups key/value pairs. To
get an actual iterator, use `begin()`. The easiest way to use this is with
range-for.

The iterator returns a `std:pair` where the `first` is the key and `second` is
the Setting & value.

This returns key/value pairs in key order regardless of insert order.

```C++
Setting & setting{Setting::setting_type::GROUP};

setting.add_child("b", 2);
setting.add_child("a", 12);
// ...

for (auto &c : setting.enumerate()) {
    std::cout << c->first << " : " << c->second.get<int>() << "\n";
}

// more manually ...

auto & e = setting.enumerate();

for (auto &iter = e.begin(); iter != e.end(); ++iter) {
    std::cout << c->first << " : " << c->second.get<int>() << "\n";
}
```