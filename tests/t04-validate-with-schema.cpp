#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <simpleConfig.hpp>

#include <string>

using namespace std::literals::string_literals;

using namespace simpleConfig;

TEST_CASE("simple test") {

    auto schema_text = "foo! : int"s;
    auto config_text = "foo : 32"s;

    auto cfg = Config();

    cfg.set_schema(schema_text);

    cfg.parse(config_text);


}

TEST_CASE("wrong key test") {

    auto schema_text = "foo! : int"s;
    auto config_text = "bar : 32"s;

    auto cfg = Config();

    cfg.set_schema(schema_text);

    CHECK_THROWS(cfg.parse(config_text));
}

TEST_CASE("missing key test") {

    auto schema_text = "foo! : int bar:int"s;
    auto config_text = "bar : 32"s;

    auto cfg = Config();

    cfg.set_schema(schema_text);

    CHECK_THROWS(cfg.parse(config_text));
}

TEST_CASE("star test") {

    auto schema_text = "foo : int, *:int"s;
    auto config_text = "bar : 32"s;

    auto cfg = Config();

    CHECK(cfg.set_schema(schema_text));

    CHECK(cfg.parse(config_text));

    config_text = "harfbuzz : true"s;

    CHECK_THROWS(cfg.parse(config_text));

}

TEST_CASE("nested schema") {
    auto schema_text = "foo : int bar :{ alpha : int * : any}"s;
    auto config_text = "foo : 32, bar : { x : 74 }"s;

    auto cfg = Config();

    CHECK(cfg.set_schema(schema_text));

    CHECK(cfg.parse(config_text)); 

    config_text = "foo : 32, bar : {}"s;
    CHECK(cfg.parse(config_text)); 

}

TEST_CASE("arrays schema") {

    SUBCASE("simple (ok)") {
        auto schema_text = "foo : [int]"s;
        auto config_text = "foo : [1, 2, 3]"s;

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));

        CHECK(cfg.parse(config_text));
    }

    SUBCASE("simple (bad)") {
        auto schema_text = "foo : [int]"s;
        auto config_text = "foo : [true]"s;

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));
        CHECK_THROWS(cfg.parse(config_text)); 
    }


}