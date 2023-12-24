#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <schema_parser.hpp>

#include <string>

using namespace std::literals::string_literals;

using namespace simpleConfig;

TEST_CASE("simple schema") {
    auto node = SchemaNode();
    auto text = "foo! : int"s;

    auto parser = SchemaParser(text, &node);

    auto results = parser.do_parse();

    CHECK(results);

    CHECK(node.subkeys.size() == 1);

    auto spec = node.subkeys.at("foo");
    CHECK(spec.required);
    CHECK(spec.vtype == ValType::INTEGER);
    CHECK(spec.name == "foo"s);
    
}

TEST_CASE("multiple schema") {
    auto node = SchemaNode();
    auto text = "foo! : int bar:float"s;

    auto parser = SchemaParser(text, &node);

    auto results = parser.do_parse();

    CHECK(results);

    CHECK(node.subkeys.size() == 2);

    auto spec = node.subkeys.at("foo");
    CHECK(spec.required);
    CHECK(spec.vtype == ValType::INTEGER);
    CHECK(spec.name == "foo"s);

    spec = node.subkeys.at("bar");
    CHECK_FALSE(spec.required);
    CHECK(spec.vtype == ValType::FLOAT);
    CHECK(spec.name == "bar"s);

}

TEST_CASE("nested schema") {
    auto node = SchemaNode();
    auto text = "foo! : int; bar:{ baz : any, * : any}"s;

    auto parser = SchemaParser(text, &node);

    auto results = parser.do_parse();

    CHECK(results);

    CHECK(node.subkeys.size() == 2);

    auto spec = node.subkeys.at("foo");
    CHECK(spec.required);
    CHECK(spec.vtype == ValType::INTEGER);
    CHECK(spec.name == "foo"s);

    spec = node.subkeys.at("bar");
    CHECK_FALSE(spec.required);
    CHECK(spec.vtype == ValType::GROUP);
    CHECK(spec.name == "bar"s);
    CHECK(spec.subkeys.size() == 2);

}