#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "parser.h"
#include "teststructures.h"

const std::string test = "[ Section1 ]\n"
                         ";first section\n"
                         "value1 = 123\n"
                         "value2 = 12.5\n"
                         "\n"
                         "value3 = string\n"
                         "[Section_2 ]\n"
                         ";second section\n"
                         "  value_1: 21\n"
                         "value_2:= 5.25\n"
                         "value___3 = sssssss; aaaaaaaa\n"
                         "[last_section]\n"
                         "str := \"test string\"\n"
                         "mult=several words string\n"
                         ";empty: \n"
                         "enum = test_enum::three   \n"
                         "arr : [1, 2, 3, \"string\"]";

BOOST_AUTO_TEST_SUITE(ParserTestSuit)

    BOOST_AUTO_TEST_CASE(ParserTest)
    {
        std::istringstream iss(test);
        ini::File<std::string> file;
        ini::parse(std::istream_iterator<ini::Line<std::string>>(iss), std::istream_iterator<ini::Line<std::string>>(), file);

        auto section_1 = file.at("Section1");
        BOOST_CHECK_EQUAL(section_1.at("value1").as<int>(), 123);
        BOOST_CHECK_EQUAL(section_1.at("value2").as<double>(), 12.5);
        BOOST_CHECK_EQUAL(section_1.at("value3").as<std::string>(), "string");
        BOOST_CHECK_EQUAL(section_1.get<std::string>("value5", "nothing"), "nothing");

        auto section_2 = file.at("Section_2");
        BOOST_CHECK_EQUAL(section_2.at("value_1").as<int>(), 21);
        BOOST_CHECK_EQUAL(section_2.at("value_2").as<double>(), 5.25);
        BOOST_CHECK_EQUAL(section_2.at("value___3").as<std::string>(), "sssssss");

        auto section_3 = file.at("last_section");
        BOOST_CHECK_EQUAL(section_3.at("str").as<std::string>(), "test string");
        BOOST_CHECK_EQUAL(section_3.at("mult").as<std::string>(), "several words string");
        BOOST_CHECK(section_3.find("empty") == section_3.end());
        BOOST_CHECK(section_3.at("enum").as<user::test_enum>() == user::test_enum::three);

        auto arr = section_3.get<std::vector<ini::Value>>("arr");
        BOOST_CHECK_EQUAL(arr[0].as<int>(), 1);
        BOOST_CHECK_EQUAL(arr[1].as<int>(), 2);
        BOOST_CHECK_EQUAL(arr[2].as<int>(), 3);
        BOOST_CHECK_EQUAL(arr[3].as<std::string>(), "string");

        for(const auto& section: file)
        {
            std::cout << "Section '" <<  section.first << "':" << std::endl;
            for(const auto& value: section.second)
                std::cout << "\t" << value.first << ": " << value.second.as<std::string>() << std::endl;
        }
    }


BOOST_AUTO_TEST_SUITE_END()