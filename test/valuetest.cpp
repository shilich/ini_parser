#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <iterator>
#include <vector>
#include "value.h"
#include "teststructures.h"

BOOST_AUTO_TEST_SUITE(ValueTestSuit)

    BOOST_AUTO_TEST_CASE(FundamentalTypeTest)
    {
        ini::Value value("12.5");

        BOOST_CHECK_EQUAL(value.as<std::string>(), "12.5");
        BOOST_CHECK_EQUAL(value.as<float>(), 12.5f);
        BOOST_CHECK_EQUAL(value.as<double>(), 12.5);
        BOOST_CHECK_EQUAL(value.as<long double>(), 12.5l);
        BOOST_CHECK_EQUAL(value.as<int>(), 12);

        BOOST_CHECK_EQUAL(ini::Value(R"("some \"string")").as<std::string>(), "some \"string");

        BOOST_CHECK_EQUAL(ini::Value().as<int>(), 0);
        BOOST_CHECK_EQUAL(ini::Value().as<std::string>("default"), "default");
    }

    BOOST_AUTO_TEST_CASE(ArrayTypeTest)
    {
        ini::Value arr("[1,2, 3, 4, 5, 6, 7, 8, 9, 10]");
        auto vec = std::move(arr.as<std::vector<int>>());
        for(size_t i = 0; i < 10; ++i)
            BOOST_CHECK_EQUAL(vec[i], i + 1);

        ini::Value str_arr(R"([string, two strings, "several strings in commas", "commas \"inside commas\" string"])");
        auto str_vec = std::move(str_arr.as<std::vector<std::string>>());
        BOOST_CHECK_EQUAL(str_vec[0], "string");
        BOOST_CHECK_EQUAL(str_vec[1], "two strings");
        BOOST_CHECK_EQUAL(str_vec[2], "several strings in commas");
        BOOST_CHECK_EQUAL(str_vec[3], "commas \"inside commas\" string");

        ini::Value mix_arr(R"([21, 5.1, string])");
        auto mix_vec = std::move(mix_arr.as<std::vector<ini::Value>>());
        BOOST_CHECK_EQUAL(mix_vec[0].as<int>(), 21);
        BOOST_CHECK_EQUAL(mix_vec[1].as<double>(), 5.1);
        BOOST_CHECK_EQUAL(mix_vec[2].as<std::string>(), "string");
    }

    BOOST_AUTO_TEST_CASE(CustomTypeTest)
    {
        ini::Value value("42");

        BOOST_REQUIRE(typeid(value.as<A>()) == typeid(A));
        BOOST_CHECK_EQUAL(value.as<A>().val, 42);
        BOOST_REQUIRE(typeid(value.as<B>()) == typeid(B));
        BOOST_CHECK_EQUAL(value.as<B>().val, 42);
        BOOST_REQUIRE(typeid(value.as<user::C>()) == typeid(user::C));
        BOOST_CHECK_EQUAL(value.as<user::C>().val, 42);
        BOOST_REQUIRE(typeid(value.as<other::D>()) == typeid(other::D));
        BOOST_CHECK_EQUAL(value.as<other::D>().val, 42);

        BOOST_CHECK(ini::Value("test_enum::four").as<user::test_enum>() == user::test_enum::four);
        BOOST_CHECK(ini::Value("test_enum::one").as<user::test_enum>() == user::test_enum::one);
    }

BOOST_AUTO_TEST_SUITE_END()