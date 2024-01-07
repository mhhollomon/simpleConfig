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

    CHECK(cfg.set_schema(schema_text));

    CHECK_FALSE(cfg.parse(config_text));
}

TEST_CASE("missing key test") {

    auto schema_text = "foo! : int bar:int"s;
    auto config_text = "bar : 32"s;

    auto cfg = Config();

    cfg.set_schema(schema_text);

    CHECK_FALSE(cfg.parse(config_text));
}

TEST_CASE("star test") {

    auto schema_text = "foo : int, *:int"s;
    auto config_text = "bar : 32"s;

    auto cfg = Config();

    CHECK(cfg.set_schema(schema_text));

    //cfg.stream_errors(std::cout);

    CHECK(cfg.parse(config_text));

    config_text = "harfbuzz : true"s;

    CHECK_FALSE(cfg.parse(config_text));

}

TEST_CASE("nested schema") {
    auto schema_text = "foo : int bar :{ alpha : int * : any}"s;
    auto config_text = "foo : 32, bar : { x : 74 }"s;

    auto cfg = Config();

    CHECK(cfg.set_schema(schema_text));

    INFO(cfg.get_errors());

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
        CHECK_FALSE(cfg.parse(config_text)); 
    }

}

TEST_CASE("arrays extended schema") {

    SUBCASE("simple (ok)") {
        auto schema_text = "foo : { _t : array _at : int}"s;
        auto config_text = "foo : [1, 2, 3]"s;

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));

        CHECK(cfg.parse(config_text));
    }

    SUBCASE("simple (bad)") {
        auto schema_text = "foo : { _t : array _at : int}"s;
        auto config_text = "foo : [true]"s;

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));
        CHECK_FALSE(cfg.parse(config_text)); 
    }

    SUBCASE("length check (ok))") {
        auto schema_text = "foo : { _t : array _at : int, _len : [2]}"s;
        auto config_text = "foo : [1, 2]"s;

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));
        CHECK(cfg.parse(config_text)); 

    }

    SUBCASE("length check (bad))") {
        auto schema_text = "foo : { _t : array _at : int, _len : [2]}"s;
        auto config_text = "foo : [12]"s;

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));
        CHECK_FALSE(cfg.parse(config_text)); 

    }
}

TEST_CASE("value limiting integers") {
    SUBCASE("scalar setting (ok)") {
        auto schema_text = "foo : { _t : int _range : [-5, 5]}"s;
        auto config_text = "foo : 3"s;

        auto cfg = Config();
        INFO(cfg.get_errors());
        CHECK(cfg.set_schema(schema_text));
        CHECK(cfg.parse(config_text)); 

    }

    SUBCASE("scalar setting (bad)") {
        auto schema_text = "foo : { _t : int _range : [-5, 5]}"s;
        auto config_text = "foo : 100"s;

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));
        CHECK_FALSE(cfg.parse(config_text)); 
        config_text = "foo : -100"s;
        CHECK_FALSE(cfg.parse(config_text)); 

    }

    SUBCASE("array setting (ok)") {
        auto schema_text = "foo : { _t : array _at : int  _range : [-5, 5]}"s;
        auto config_text = "foo : [1 3 4 0 -5]"s;

        auto cfg = Config();
        INFO(cfg.get_errors());
        CHECK(cfg.set_schema(schema_text));
        CHECK(cfg.parse(config_text)); 

    }

    SUBCASE("array setting (bad)") {
        auto schema_text = "foo : { _t : array; _at : int;  _range : [-5, 5]}"s;
        auto config_text = "foo : [1 3 4 0 100 ]"s;

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));
        CHECK_FALSE(cfg.parse(config_text)); 
        config_text = "foo : [1 3 -100 0 2 ]"s;
        CHECK_FALSE(cfg.parse(config_text)); 

    }

}

TEST_CASE("value limiting floats") {
    SUBCASE("scalar setting (ok)") {
        auto schema_text = "foo : { _t : float _range : [-5, 5.12]}"s;
        auto config_text = "foo : 3.3"s;

        auto cfg = Config();
        INFO(cfg.get_errors());
        CHECK(cfg.set_schema(schema_text));
        CHECK(cfg.parse(config_text)); 

    }

    SUBCASE("scalar setting (bad)") {
        auto schema_text = "foo : { _t : float _range : [-5, 5.1]}"s;
        auto config_text = "foo : 100.0"s;

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));
        CHECK_FALSE(cfg.parse(config_text)); 
        config_text = "foo : -100.0"s;
        CHECK_FALSE(cfg.parse(config_text)); 

    }

    SUBCASE("array setting (ok)") {
        auto schema_text = "foo : { _t : array _at : float  _range : [-5.6, 5.0]}"s;
        auto config_text = "foo : [1.0 3.0 4.0 0.0 -5.6]"s;

        auto cfg = Config();
        INFO(cfg.get_errors());
        CHECK(cfg.set_schema(schema_text));
        CHECK(cfg.parse(config_text)); 

    }

    SUBCASE("array setting (bad)") {
        auto schema_text = "foo : { _t : array; _at : float;  _range : [-5.6, 5.0]}"s;
        auto config_text = "foo : [1.0 3.1415926 4.0 0.0 100.0 ]"s;

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));
        CHECK_FALSE(cfg.parse(config_text)); 
        config_text = "foo : [1.0 3.0 -100.0 0.0 2.0 ]"s;
        CHECK_FALSE(cfg.parse(config_text)); 

    }

}

TEST_CASE("Defaults") {
    SUBCASE("simple int with default (ok)") {
        auto schema_text = "a : int b : { _t : int _d : 3}"s;
        auto config_text = "a = 42;"s;

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));
        CHECK(cfg.parse(config_text));

        auto &b = cfg.get_settings().at("b");

        CHECK(b.get<int>() == 3);

    }
}

TEST_CASE("faults seen") {
    SUBCASE("* key") {
        const std::string  schema_text = R"DELIM(
server! : {
	interface : { _t : string, _d : "127.0.0.1"; },
	port : { _t : int, _d : 8888 };
}

launch! : {
	*! : string
}
)DELIM";

    const std::string config_text = R"DELIM(
server: {
        port = 7777;
}

launch : {

        gwenview = "/usr/bin/gwenview"
}

)DELIM";

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));

        INFO(cfg.stream_errors(std::cout));
        CHECK(cfg.parse(config_text));


    }
}

TEST_CASE("array with group") {
    SUBCASE("ok") {
        auto schema_text = " b : { _t : array _at : group  a : string b : float}"s;
        auto config_text = "b = [ { a = \"h\", b = 42.0 } ];"s;

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));
        INFO(cfg.get_errors());
        CHECK(cfg.parse(config_text));       
    }
    SUBCASE("bad") {
        auto schema_text = " b : { _t : array _at : group  a : string b : float}"s;
        auto config_text = "b = [ { a = 5, b = 42.0 } ];"s;

        auto cfg = Config();

        CHECK(cfg.set_schema(schema_text));
        INFO(cfg.get_errors());
        CHECK_FALSE(cfg.parse(config_text));       
    }
}