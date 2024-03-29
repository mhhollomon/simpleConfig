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

    auto &parser = setup_schema_parser(text);

    auto results = parser.do_parse();

    auto *node = parser.schema;

    INFO(std::cout << parser.errors);

    CHECK(results);

    CHECK(node->subkeys.size() == 1);

    auto &spec = node->subkeys.at("foo");
    CHECK(spec.required);
    CHECK(spec.vtype == ValType::INTEGER);
    CHECK(spec.name == "foo"s);
    
}

TEST_CASE("check names") {

    SUBCASE("star") {
        auto text = "* : int"s;

        auto &parser = setup_schema_parser(text);
        CHECK(parser.do_parse());
    }

    SUBCASE("underbar") {
        auto text = "_x : int"s;

        auto &parser = setup_schema_parser(text);
        CHECK_FALSE(parser.do_parse());
    }

    SUBCASE("dash at start (bad)") {
        auto text = "-x : int"s;

        auto &parser = setup_schema_parser(text);
        CHECK_FALSE(parser.do_parse());
    }
    
    SUBCASE("dash in middle (ok)") {
        auto text = "x-y : int"s;

        auto &parser = setup_schema_parser(text);
        CHECK(parser.do_parse());
    }

    SUBCASE("digit at start (bad)") {
        auto text = "1st-digit : int"s;

        auto &parser = setup_schema_parser(text);
        CHECK_FALSE(parser.do_parse());
    }


    SUBCASE("double underbar in middle (ok)") {
        auto text = "under__bar : int"s;

        auto &parser = setup_schema_parser(text);
        CHECK(parser.do_parse());
    }

}

TEST_CASE("multiple schema") {
    auto text = "foo! : int bar:float"s;

    auto &parser = setup_schema_parser(text);

    auto results = parser.do_parse();


    CHECK(results);

    auto *node = parser.schema;

    CHECK(node->subkeys.size() == 2);

    auto &spec = node->subkeys.at("foo");
    CHECK(spec.required);
    CHECK(spec.vtype == ValType::INTEGER);
    CHECK(spec.name == "foo"s);

    auto &spec2 = node->subkeys.at("bar");
    CHECK_FALSE(spec2.required);
    CHECK(spec2.vtype == ValType::FLOAT);
    CHECK(spec2.name == "bar"s);

}

TEST_CASE("nested schema") {
    auto text = "foo! : int; bar:{ baz : any, * : any}"s;

    auto &parser = setup_schema_parser(text);

    auto results = parser.do_parse();

    CHECK(results);

    auto *node = parser.schema;

    CHECK(node->subkeys.size() == 2);

    auto &spec = node->subkeys.at("foo");
    CHECK(spec.required);
    CHECK(spec.vtype == ValType::INTEGER);
    CHECK(spec.name == "foo"s);

    auto &spec2 = node->subkeys.at("bar");
    CHECK_FALSE(spec2.required);
    CHECK(spec2.vtype == ValType::GROUP);
    CHECK(spec2.name == "bar"s);
    CHECK(spec2.subkeys.size() == 2);

}

TEST_CASE("lists") {

    SUBCASE("simple") {
        auto text = "key : ()"s;
        auto &parser = setup_schema_parser(text);

        CHECK(parser.do_parse());
        auto *node = parser.schema;
        auto &spec = node->subkeys.at("key");
        CHECK(spec.vtype == ValType::LIST);
    }

    SUBCASE("stuff inside (bad)") {
        auto text = "key : ( int )"s;
        auto &parser = setup_schema_parser(text);
        CHECK_FALSE(parser.do_parse());
    }
}

TEST_CASE("arrays") {

    SUBCASE("simple") {
        auto text = "key : [int]"s;
        auto &parser = setup_schema_parser(text);

        CHECK(parser.do_parse());

        auto * node = parser.schema;
        auto &spec = node->subkeys.at("key");
        CHECK(spec.vtype == ValType::ARRAY);
        CHECK(spec.array_type == ValType::INTEGER);
    }

    SUBCASE("missing type (bad)") {
        auto text = "key : [ /* nothing here */ ]"s;
        auto &parser = setup_schema_parser(text);
        CHECK_FALSE(parser.do_parse());
    }

}

TEST_CASE("simple extended") {
    SUBCASE("just type") {
        auto text = R"DELIM(
            key : { _t : int },
            key_string : { _type : string };
            key_bool : {_t : bool }
            k-float : { _type : float },
            k-any : {_t:
                any}
        )DELIM"s;
        auto &parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);
        CHECK(parser.do_parse());
    }

    SUBCASE("required (ok)") {
        auto text = "key : { _t : int, _r : tRuE}"s;
        auto &parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);

        CHECK(parser.do_parse());

        auto *node = parser.schema;

        CHECK(node->subkeys["key"].required);

    }

    SUBCASE("required (bad)") {
        // can't use the '!' with '_required'
        auto text = "key! : { _t : int; _required : tRuE}"s;
        auto &parser = setup_schema_parser(text);
        CHECK_FALSE(parser.do_parse());
    }

    SUBCASE("group keys") {
        // note this also shows you can mix and match
        // concise and extended formats.
        auto text = "key : { _t : group, k2 : int *! : { _t : any }}"s;
        auto &parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);

        CHECK(parser.do_parse());

        auto *node = parser.schema;

        CHECK(node->subkeys["key"].vtype == ValType::GROUP);

    }

    SUBCASE("array keys") {
        auto text = "key : { _t : array, _at : int }"s;
        auto &parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);

        CHECK(parser.do_parse());

    }

    SUBCASE("array type (bad)") {
        auto text = "key : { _t : group, _arraytype : int }"s;
        auto &parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);

        CHECK_FALSE(parser.do_parse());

    }

}

TEST_CASE("array length") {

    SUBCASE("one entry") {
        auto text = "key : { _t : array, _arrtype : int, _len : [3] }"s;
        auto parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);

        CHECK(parser.do_parse());

        auto &kn = parser.schema->subkeys["key"];

        CHECK(kn.length == Range<long>(3, 3));

    }
    
    SUBCASE("one entry (bad)") {
        auto text = "key : { _t : array, _arrtype : int, _len : [-2] }"s;
        auto &parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);

        CHECK_FALSE(parser.do_parse());
    }

    SUBCASE("two entries") {
        auto text = "key : { _t : array, _arrtype : int, _len : [3, 5] }"s;
        auto &parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);

        CHECK(parser.do_parse());

        auto &kn = parser.schema->subkeys["key"];

        CHECK(kn.length == Range<long>(3, 5));

    }

    SUBCASE("two entries [bad]") {
        auto text = "key : { _t : array, _arrtype : int, _len : [6, 2] }"s;
        auto &parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);

        CHECK_FALSE(parser.do_parse());

    }

    SUBCASE("_len with non-array [bad]") {
        auto text = "key : { _t : int,  _len : [6, 2] }"s;
        auto &parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);

        CHECK_FALSE(parser.do_parse());

    }
}

TEST_CASE("Defaults") {
    SUBCASE("int") {
        auto text = "key : { _t : int, _d : 3 }"s;
        auto &parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);

        CHECK(parser.do_parse());

    }
    SUBCASE("int with range (ok)") {
        auto text = "key : { _t : int, _range : [ 1, 6 ], _d : 3 }"s;
        auto &parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);

        CHECK(parser.do_parse());

    }
    SUBCASE("int with range (bad)") {
        auto text = "key : { _t : int, _range : [ 1, 6 ], _d : -100 }"s;
        auto &parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);

        CHECK_FALSE(parser.do_parse());

    }

    SUBCASE("default on array (bad)") {
        auto text = "key : { _t : array, _at : int, _d : -100 }"s;
        auto &parser = setup_schema_parser(text);
        INFO(std::cout << parser.errors);

        CHECK_FALSE(parser.do_parse());

    }
}

TEST_CASE("enums") {
    SUBCASE("simple int") {
        auto text = "key : { _t : int, _enum : [1 5 10] }"s;
        auto &parser = setup_schema_parser(text);
        INFO(parser.errors);

        CHECK(parser.do_parse());

    }
    SUBCASE("bad int") {
        auto text = "key : { _t : int, _enum : [1 5 23.1] }"s;
        auto &parser = setup_schema_parser(text);
        INFO(parser.errors);

        CHECK_FALSE(parser.do_parse());

    }
    SUBCASE("simple float") {
        auto text = "key : { _t : float, _enum : [1.0 5.2 -10.0] }"s;
        auto &parser = setup_schema_parser(text);
        INFO(parser.errors);

        CHECK(parser.do_parse());

    }
    SUBCASE("bad int") {
        auto text = "key : { _t : int, _enum : [1.0 -5 23.1] }"s;
        auto &parser = setup_schema_parser(text);
        INFO(parser.errors);

        CHECK_FALSE(parser.do_parse());

    }

    SUBCASE("simple string") {
        auto text = "key : { _t : string, _enum : [\"a\" \"b\" \"yellow\"] }"s;
        auto &parser = setup_schema_parser(text);
        INFO(parser.errors);

        CHECK(parser.do_parse());

    }
    SUBCASE("bad string") {
        auto text = "key : { _t : string, _enum : [\"a\" \"b\" 42] }"s;
        auto &parser = setup_schema_parser(text);
        INFO(parser.errors);

        CHECK_FALSE(parser.do_parse());

    }

    SUBCASE("simple array of strings") {
        auto text = "key : { _t : array, _at : string, _enum : [\"a\" \"b\" \"yellow\"] }"s;
        auto &parser = setup_schema_parser(text);
        INFO(parser.errors);

        CHECK(parser.do_parse());

    }

}
