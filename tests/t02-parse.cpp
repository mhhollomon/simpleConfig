#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <simple_config.hpp>

#include <string>

using namespace std::literals::string_literals;

TEST_CASE("compiles") {
    Configinator5000::Config cfg;
    CHECK(true);

    std::string input = R"DELIM( port 
= 7777; )DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b == true);

    auto & s = cfg.get_settings();

    CHECK(s.count() == 1);
    CHECK(s.exists("port"));
    CHECK(s.is_group());
    CHECK(s.at("port").is_integer());
    CHECK(s.at("port").get<int>() == 7777);
}

TEST_CASE("string") {
    Configinator5000::Config cfg;
    CHECK(true);

    std::string input = R"DELIM( port = "hello"; )DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b);

    auto & s = cfg.get_settings();

    CHECK(s.count() == 1);
    CHECK(s.exists("port"));
    CHECK(s.is_group());
    CHECK(s.at("port").is_string());
    CHECK(s.at("port").get<std::string>() == "hello"s);

    // Multi strings

    input = R"DELIM( port = "hel"
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
    Configinator5000::Config cfg;

    std::string input = R"DELIM( port = 42.0; )DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b);

    auto & s = cfg.get_settings();

    CHECK(s.count() == 1);
    CHECK(s.exists("port"));
    CHECK(s.is_group());
    CHECK(s.at("port").is_float());
    CHECK(s.at("port").get<double>() == 42.0);
}

TEST_CASE("bool") {
    Configinator5000::Config cfg;

    std::string input = R"DELIM( 
        b1 = TRUE;
        b2 : FALSE
        b3 = TruE,
        b4 : fALse
 )DELIM"s;

    bool b = cfg.parse(input);

    CHECK(b);

    auto & s = cfg.get_settings();

    CHECK(s.count() == 4);
    CHECK(s.exists("b1"));
    CHECK(s.at("b1").is_boolean());
    CHECK(s.at("b1").get<bool>() == true);
    CHECK(s.at("b2").is_boolean());
    CHECK(s.at("b2").get<bool>() == false);
    CHECK(s.at("b3").is_boolean());
    CHECK(s.at("b3").get<bool>() == true);
    CHECK(s.at("b4").is_boolean());
    CHECK(s.at("b4").get<bool>() == false);
}

TEST_CASE("Nested Group") {
    Configinator5000::Config cfg;

    std::string input = R"DELIM( a = {
    a1 = true;
    a2 = 3;
    }
b = {
    b1 = {
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
    Configinator5000::Config cfg;

    std::string input = R"DELIM(
mylist = (
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
    Configinator5000::Config cfg;

    std::string input = R"DELIM( good = [ 1, 2 ] )DELIM"s;

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
    CHECK_FALSE(cfg.parse("bad = [1, 43.0]"));

    // group
    CHECK_FALSE(cfg.parse("bad = [1, { a : 2 }]"));

    // list
    CHECK_FALSE(cfg.parse("bad : [ 42.0 ( 1, 2 ) ]"));
}

TEST_CASE("errors") {
    Configinator5000::Config cfg;

    std::string input = R"DELIM( bad = $%^ )DELIM"s;

    CHECK_FALSE(cfg.parse(input));

    std::stringstream buf{};

    cfg.stream_errors(buf);

    CHECK(buf.str() == "line 0 : Expecting a value\nline 0 : Not at end of input!\n"s);
}
