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

TEST_CASE("check names") {

    SUBCASE("star") {
        auto node = SchemaNode();
        auto text = "* : int"s;

        auto parser = SchemaParser(text, &node);
        CHECK(parser.do_parse());
    }

    SUBCASE("underbar") {
        auto node = SchemaNode();
        auto text = "_x : int"s;

        auto parser = SchemaParser(text, &node);
        CHECK(parser.do_parse());
    }

    SUBCASE("dash at start (bad)") {
        auto node = SchemaNode();
        auto text = "-x : int"s;

        auto parser = SchemaParser(text, &node);
        CHECK_FALSE(parser.do_parse());
    }
    SUBCASE("dash in middle (ok)") {
        auto node = SchemaNode();
        auto text = "x-y : int"s;

        auto parser = SchemaParser(text, &node);
        CHECK(parser.do_parse());
    }

    SUBCASE("digit at start (bad)") {
        auto node = SchemaNode();
        auto text = "1st-digit : int"s;

        auto parser = SchemaParser(text, &node);
        CHECK_FALSE(parser.do_parse());
    }

    SUBCASE("digit after underbar at start (bad)") {
        auto node = SchemaNode();
        auto text = "_3-blind_mice : int"s;

        auto parser = SchemaParser(text, &node);
        CHECK_FALSE(parser.do_parse());
    }

    SUBCASE("dash after underbar at start (bad)") {
        auto node = SchemaNode();
        auto text = "_-what : int"s;

        auto parser = SchemaParser(text, &node);
        CHECK_FALSE(parser.do_parse());
    }

    SUBCASE("double underbar at start (bad)") {
        auto node = SchemaNode();
        auto text = "__underbar : int"s;

        auto parser = SchemaParser(text, &node);
        CHECK_FALSE(parser.do_parse());
    }

    SUBCASE("double underbar in middle (ok)") {
        auto node = SchemaNode();
        auto text = "under__bar : int"s;

        auto parser = SchemaParser(text, &node);
        CHECK(parser.do_parse());
    }

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

TEST_CASE("lists") {

    SUBCASE("simple") {
        auto node = SchemaNode();
        auto text = "key : ()"s;
        auto parser = SchemaParser(text, &node);

        CHECK(parser.do_parse());

        auto spec = node.subkeys.at("key");
        CHECK(spec.vtype == ValType::LIST);
    }

    SUBCASE("stuff inside (bad)") {
        auto node = SchemaNode();
        auto text = "key : ( int )"s;
        auto parser = SchemaParser(text, &node);
        CHECK_FALSE(parser.do_parse());
    }
}

TEST_CASE("arrays") {

    SUBCASE("simple") {
        auto node = SchemaNode();
        auto text = "key : [int]"s;
        auto parser = SchemaParser(text, &node);

        CHECK(parser.do_parse());

        auto spec = node.subkeys.at("key");
        CHECK(spec.vtype == ValType::ARRAY);
        CHECK(spec.array_type == ValType::INTEGER);
    }

    SUBCASE("missing type (bad)") {
        auto node = SchemaNode();
        auto text = "key : [ /* nothing here */ ]"s;
        auto parser = SchemaParser(text, &node);
        CHECK_FALSE(parser.do_parse());
    }

    SUBCASE("group type") {
        auto node = SchemaNode();
        auto text = "key : [{x : int }]"s;
        auto parser = SchemaParser(text, &node);

        auto results = parser.do_parse();

        if (!results)
            std::cout << parser.errors;

        CHECK(results);

        auto spec = node.subkeys.at("key");
        CHECK(spec.vtype == ValType::ARRAY);
        CHECK(spec.array_type == ValType::GROUP);
    }


}