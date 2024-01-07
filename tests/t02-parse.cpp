#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <simpleConfig.hpp>

#include <string>

using namespace std::literals::string_literals;

TEST_CASE("compiles") {
    simpleConfig::Config cfg;
    CHECK(true);

    std::string input = R"DELIM( port 
: 7777; )DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b == true);

    auto & s = cfg.get_settings();

    CHECK(s.count() == 1);
    CHECK(s.exists("port"));
    CHECK(s.is_group());
    CHECK(s.at("port").is_integer());
    CHECK(s.at("port").get<int>() == 7777);
}

TEST_CASE("bad names") {
    simpleConfig::Config cfg;

    std::string input = "-port : 42"s;
    CHECK_FALSE(cfg.parse(input));

    input = "_port : 42"s;
    CHECK_FALSE(cfg.parse(input));

    input = "* : 42"s;
    CHECK_FALSE(cfg.parse(input));

    input = "1st_key : 42"s;
    CHECK_FALSE(cfg.parse(input));

}

TEST_CASE("string") {
    simpleConfig::Config cfg;
    CHECK(true);

    std::string input = R"DELIM( port = "hello")DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b);

    auto & s = cfg.get_settings();

    CHECK(s.count() == 1);
    CHECK(s.exists("port"));
    CHECK(s.is_group());
    CHECK(s.at("port").is_string());
    CHECK(s.at("port").get<std::string>() == "hello"s);

    // Multi strings

    input = R"DELIM( port : "hel"
"lo"
)DELIM"s;

    b = cfg.parse(input);
    CHECK(b);

    auto & s2 = cfg.get_settings();

    CHECK(s2.count() == 1);
    CHECK(s2.exists("port"));
    CHECK(s2.at("port").is_string());
    CHECK(s2.at("port").get<std::string>() == "hello"s);

}

TEST_CASE("float") {
    simpleConfig::Config cfg;

    std::string input = R"DELIM( port-number = 42.0; )DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b);

    auto & s = cfg.get_settings();

    CHECK(s.count() == 1);
    CHECK(s.exists("port-number"));
    CHECK(s.is_group());
    CHECK(s.at("port-number").is_float());
    CHECK(s.at("port-number").get<double>() == 42.0);
}

TEST_CASE("bool") {
    simpleConfig::Config cfg;

    std::string input = R"DELIM( 
        b1: TRUE;
        b2 : FALSE
        b3 : TruE,
        b4 : fALse
        b5 : true
 )DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b);

    auto & s = cfg.get_settings();

    CHECK(s.count() == 5);
    CHECK(s.exists("b1"));
    CHECK(s.at("b1").is_boolean());
    CHECK(s.at("b1").get<bool>() == true);
    CHECK(s.at("b2").is_boolean());
    CHECK(s.at("b2").get<bool>() == false);
    CHECK(s.at("b3").is_boolean());
    CHECK(s.at("b3").get<bool>() == true);
    CHECK(s.at("b4").is_boolean());
    CHECK(s.at("b4").get<bool>() == false);
    CHECK(s.at("b5").is_boolean());
    CHECK(s.at("b5").get<bool>() == true);
}

TEST_CASE("Nested Group") {
    simpleConfig::Config cfg;

    std::string input = R"DELIM( a :{
    a1 : true;
    a2 :3;
    }
b : {
    b1 : {
        bb1 : "foo",
    },
}
)DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b);

    auto & s = cfg.get_settings();
    CHECK(s.count() == 2);
    CHECK(s.exists("a"));
    CHECK(s.exists("b"));

    auto &a = s.at("a");
    CHECK(a.count() == 2);
    CHECK(a.at("a1").get<bool>() == true);
    CHECK(a.at("a2").get<int>() == 3);

    
    CHECK(s.at("b").at("b1").at("bb1").get<std::string>() == "foo"s);

}

TEST_CASE("list") {
    simpleConfig::Config cfg;

    std::string input = R"DELIM(
mylist : (
    "A",
    { a : 3 b: "long" " string" }
    [ 1, 2 ]
)
)DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b);

    auto & s = cfg.get_settings();

    CHECK(s.count() == 1);
    CHECK(s.exists("mylist"));
    auto & l = s.at("mylist");
    CHECK(l.is_list());
    CHECK(l.count() == 3);

    CHECK(l.at(0).get<std::string>() == "A"s);

    auto &g = l.at(1);
    CHECK(g.at("a").get<int>() == 3);
    CHECK(g.at("b").get<std::string>() == "long string");

    auto &ary = l.at(2);
    CHECK(ary.at(0).get<int>() == 1);
    CHECK(ary.at(1).get<int> ()== 2);

}

TEST_CASE("array") {
    simpleConfig::Config cfg;

    std::string input = R"DELIM( good : [ 1, 2 ] )DELIM"s;

    CHECK(cfg.parse(input));

    auto & s = cfg.get_settings();

    CHECK(s.count() == 1);
    CHECK(s.exists("good"));

    auto &good = s.at("good");
    CHECK(good.count() == 2);
    CHECK(good.at(0).get<int>() == 1);
    CHECK(good.at(1).get<int>() == 2);

    int i = 1;
    for (auto &x : good) {
        CHECK(x.get<int>() == i++);
    }

    // mixed
    CHECK_FALSE(cfg.parse("bad : [1, 43.0]"));

    // group
    CHECK_FALSE(cfg.parse("bad : [1, { a : 2 }]"));

    // list
    CHECK_FALSE(cfg.parse("bad : [ 42.0 ( 1, 2 ) ]"));
}

TEST_CASE("array with subgroups") {
    SUBCASE("simple (ok)") {
        simpleConfig::Config cfg;

        std::string input = R"DELIM( good : [ { a: 1 }] )DELIM"s;
        INFO(cfg.get_errors());
        CHECK(cfg.parse(input));
        auto & s = cfg.get_settings();
        auto &good = s.at("good");
        CHECK(good.count() == 1);
        auto &a = good.at(0);
        CHECK(a.is_group());
        CHECK(a.at("a").get<int>() == 1);

    }
    SUBCASE("multiple (ok)") {
        simpleConfig::Config cfg;

        std::string input = R"DELIM( good : [ { a: 1 }, { b: 42}] )DELIM"s;
        INFO(cfg.get_errors());
        CHECK(cfg.parse(input));
        auto & s = cfg.get_settings();
        auto &good = s.at("good");
        CHECK(good.count() == 2);
        auto &a = good.at(0);
        CHECK(a.is_group());
        CHECK(a.at("a").get<int>() == 1);
        auto &b = good.at(1);
        CHECK(b.is_group());
        CHECK(b.at("b").get<int>() == 42);

    }

    SUBCASE("Different types (bad)") {
         simpleConfig::Config cfg;

        std::string input = R"DELIM( bad : [ { a: 1 }, 57.7 ] )DELIM"s;
        CHECK_FALSE(cfg.parse(input));
       
    }
}

TEST_CASE("errors") {
    simpleConfig::Config cfg;

    std::string input = R"DELIM( bad : $%^ )DELIM"s;

    CHECK_FALSE(cfg.parse(input));

    std::stringstream buf{};

    cfg.stream_errors(buf);

    CHECK(buf.str() == "line 0 : Config: Expecting a value\nline 0 : Config: Not at end of input!\n"s);
}


TEST_CASE("at_path") {
    //These really should be in t01, but it is easier to let the parser
    // build the nested structure.
    SUBCASE("string") {
            simpleConfig::Config cfg;

            std::string input = R"DELIM( a : { b = 3 }, c = "hello" )DELIM"s;

            CHECK(cfg.parse(input));

            std::vector x = {"a"s,"b"s};

            CHECK(cfg.get_settings().at_path(x).get<int>() == 3);

    }
    SUBCASE("array index") {
            simpleConfig::Config cfg;

            std::string input = R"DELIM( a : { b = [1 ,2 ,5] }, c = "hello" )DELIM"s;

            CHECK(cfg.parse(input));

            CHECK(cfg.at_path({"a","b","2"}).get<int>() == 5);

            CHECK(cfg.at_path("a.b.[2]"s).get<int>() == 5);

    }
}

TEST_CASE("Comments") {
    SUBCASE("Hash Comment") {
        simpleConfig::Config cfg;

        std::string input = R"DELIM( a : { b = 3 }, 
# This is a hash comment
c = "hello" )DELIM"s;

        CHECK(cfg.parse(input));

        CHECK(cfg.at_path("c").get<std::string>() == "hello");

    }

    SUBCASE("Line Comment") {
        simpleConfig::Config cfg;

        std::string input = R"DELIM( a : { b = 3 }, 
// This is a c++ line comment
c = "hello" )DELIM"s;

        CHECK(cfg.parse(input));

        CHECK(cfg.at_path("c").get<std::string>() == "hello");

    }
    
    SUBCASE("Block Comment") {
        simpleConfig::Config cfg;

        std::string input = R"DELIM( a : { b /* yup */ = 3 }, 
/* 
    This is a c block comment
*/

c = "hello"
)DELIM"s;

        CHECK(cfg.parse(input));
        CHECK(cfg.get_settings().at_path("a.b").get<int>() == 3);
        CHECK(cfg.at_path("c").get<std::string>() == "hello");

    }
}

TEST_CASE("string escapes") {
    SUBCASE("backslash simple") {
        simpleConfig::Config cfg;

        std::string input = R"DELIM( 
a : "a\f\n\\b"
)DELIM"s;

        CHECK(cfg.parse(input));
        CHECK(cfg.at_path("a").get<std::string>() == "a\f\n\\b");

    }

    SUBCASE("backslash hex") {
        simpleConfig::Config cfg;

        std::string input = R"DELIM( 
a : "a\x20b"
)DELIM"s;

        CHECK(cfg.parse(input));
        CHECK(cfg.at_path("a").get<std::string>() == "a b");

    }


}
