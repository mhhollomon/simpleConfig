#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <schema_parser.hpp>

#include <string>

using namespace std::literals::string_literals;

using namespace simpleConfig;

SchemaParser &setup_schema_parser(const std::string &text) {
    return *(new SchemaParser(
        text,
        new SchemaNode{},
        *(new error_list{})
    ));
}

TEST_CASE("simple schema") {
    auto text = "foo! : int"s;

    auto parser = setup_schema_parser(text);

    auto results = parser.do_parse();

    auto *node = parser.schema;

    CHECK(results);

    CHECK(node->subkeys.size() == 1);

    auto spec = node->subkeys.at("foo");
    CHECK(spec.required);
    CHECK(spec.vtype == ValType::INTEGER);
    CHECK(spec.name == "foo"s);
    
}

TEST_CASE("check names") {

    SUBCASE("star") {
        auto text = "* : int"s;

        auto parser = setup_schema_parser(text);
        CHECK(parser.do_parse());
    }

    SUBCASE("underbar") {
        auto text = "_x : int"s;

        auto parser = setup_schema_parser(text);
        CHECK(parser.do_parse());
    }

    SUBCASE("dash at start (bad)") {
        auto text = "-x : int"s;

        auto parser = setup_schema_parser(text);
        CHECK_FALSE(parser.do_parse());
    }
    
    SUBCASE("dash in middle (ok)") {
        auto text = "x-y : int"s;

        auto parser = setup_schema_parser(text);
        CHECK(parser.do_parse());
    }

    SUBCASE("digit at start (bad)") {
        auto text = "1st-digit : int"s;

        auto parser = setup_schema_parser(text);
        CHECK_FALSE(parser.do_parse());
    }

    SUBCASE("digit after underbar at start (bad)") {
        auto text = "_3-blind_mice : int"s;

        auto parser = setup_schema_parser(text);
        CHECK_FALSE(parser.do_parse());
    }

    SUBCASE("dash after underbar at start (bad)") {
        auto text = "_-what : int"s;

        auto parser = setup_schema_parser(text);
        CHECK_FALSE(parser.do_parse());
    }

    SUBCASE("double underbar at start (bad)") {
        auto text = "__underbar : int"s;

        auto parser = setup_schema_parser(text);
        CHECK_FALSE(parser.do_parse());
    }

    SUBCASE("double underbar in middle (ok)") {
        auto text = "under__bar : int"s;

        auto parser = setup_schema_parser(text);
        CHECK(parser.do_parse());
    }

}

TEST_CASE("multiple schema") {
    auto text = "foo! : int bar:float"s;

    auto parser = setup_schema_parser(text);

    auto results = parser.do_parse();


    CHECK(results);

    auto *node = parser.schema;

    CHECK(node->subkeys.size() == 2);

    auto spec = node->subkeys.at("foo");
    CHECK(spec.required);
    CHECK(spec.vtype == ValType::INTEGER);
    CHECK(spec.name == "foo"s);

    spec = node->subkeys.at("bar");
    CHECK_FALSE(spec.required);
    CHECK(spec.vtype == ValType::FLOAT);
    CHECK(spec.name == "bar"s);

}

TEST_CASE("nested schema") {
    auto text = "foo! : int; bar:{ baz : any, * : any}"s;

    auto parser = setup_schema_parser(text);

    auto results = parser.do_parse();

    CHECK(results);

    auto *node = parser.schema;

    CHECK(node->subkeys.size() == 2);

    auto spec = node->subkeys.at("foo");
    CHECK(spec.required);
    CHECK(spec.vtype == ValType::INTEGER);
    CHECK(spec.name == "foo"s);

    spec = node->subkeys.at("bar");
    CHECK_FALSE(spec.required);
    CHECK(spec.vtype == ValType::GROUP);
    CHECK(spec.name == "bar"s);
    CHECK(spec.subkeys.size() == 2);

}

TEST_CASE("lists") {

    SUBCASE("simple") {
        auto text = "key : ()"s;
        auto parser = setup_schema_parser(text);

        CHECK(parser.do_parse());
        auto *node = parser.schema;
        auto spec = node->subkeys.at("key");
        CHECK(spec.vtype == ValType::LIST);
    }

    SUBCASE("stuff inside (bad)") {
        auto text = "key : ( int )"s;
        auto parser = setup_schema_parser(text);
        CHECK_FALSE(parser.do_parse());
    }
}

TEST_CASE("arrays") {

    SUBCASE("simple") {
        auto text = "key : [int]"s;
        auto parser = setup_schema_parser(text);

        CHECK(parser.do_parse());

        auto * node = parser.schema;
        auto spec = node->subkeys.at("key");
        CHECK(spec.vtype == ValType::ARRAY);
        CHECK(spec.array_type == ValType::INTEGER);
    }

    SUBCASE("missing type (bad)") {
        auto text = "key : [ /* nothing here */ ]"s;
        auto parser = setup_schema_parser(text);
        CHECK_FALSE(parser.do_parse());
    }

}