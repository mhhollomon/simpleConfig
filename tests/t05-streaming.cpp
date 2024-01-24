#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <simpleConfig.hpp>

#include <string>
#include <sstream>

TEST_CASE("Streaming") {
    SUBCASE("Toplevel group") {

        simpleConfig::Config cfg;
        std::string input = R"DELIM(port : "hello"
)DELIM"s;

        REQUIRE(cfg.parse(input));
        auto & s = cfg.get_settings();
        std::stringstream ss;
        s.stream_setting(ss, "");
        CHECK(ss.str() == input);

    }

    SUBCASE("Toplevel group") {

        simpleConfig::Config cfg;
        std::string input = R"DELIM(port : "hello",
subgroup : {
  a : 1,
  b : 42.3
}
)DELIM"s;

        REQUIRE(cfg.parse(input));
        auto & s = cfg.get_settings();
        std::stringstream ss;
        s.stream_setting(ss, "");
        CHECK(ss.str() == input);

    }

    SUBCASE("Toplevel array") {

        simpleConfig::Config cfg;
        std::string input = R"DELIM(a : "hello",
array : [ "a", "b", "c" ]
)DELIM"s;

        REQUIRE(cfg.parse(input));
        auto & s = cfg.get_settings();
        std::stringstream ss;
        s.stream_setting(ss, "");
        CHECK(ss.str() == input);

    }
    SUBCASE("array with nested group") {

        simpleConfig::Config cfg;
        // Not really happy with this, but okay.
        std::string input = R"DELIM(a : "hello",
array : [
  {
    a : 1
  },
  {
    b : 2
  },
  {
    c : 3
  } ]
)DELIM"s;

        REQUIRE(cfg.parse(input));
        auto & s = cfg.get_settings();
        std::stringstream ss;
        s.stream_setting(ss, "");
        CHECK(ss.str() == input);

    }

    SUBCASE("sub-sub group") {

        simpleConfig::Config cfg;
        std::string input = R"DELIM(port : "hello",
subgroup : {
  a : 1,
  b : {
    arg1 : "This is a long string",
    arg2 : true
  },
  c : 42.3
}
)DELIM"s;

        REQUIRE(cfg.parse(input));
        auto & s = cfg.get_settings();
        std::stringstream ss;
        s.stream_setting(ss, "");
        CHECK(ss.str() == input);

    }

    SUBCASE("list") {

        simpleConfig::Config cfg;
        std::string input = R"DELIM(port : "hello",
subgroup : {
  a : 1,
  b : ( 12, "hello",
     {
      arg1 : "This is a long string",
      arg2 : true
    }, "c" )
}
)DELIM"s;

        REQUIRE(cfg.parse(input));
        auto & s = cfg.get_settings();
        std::stringstream ss;
        s.stream_setting(ss, "");
        CHECK(ss.str() == input);

    }
}