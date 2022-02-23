#ifndef INI_SYNAX_H
#define INI_SYNAX_H

#include <regex>

namespace ini
{

namespace syntax
{

template <typename CharT, typename Traits = std::char_traits<CharT>>
struct value_regex_traits {};

template <>
struct value_regex_traits<char, std::char_traits<char>>
{
const std::regex comma_regex{R"(\s*\"(((\\\")|[^\"])*)\"\s*)"};
const std::regex spaces_regex{R"(\s*(.*\S)\s*)"};
const std::regex array_regex{R"(\[([^\]]*)\])"};
};

template <>
struct value_regex_traits<wchar_t, std::char_traits<wchar_t>>
{
const std::wregex comma_regex{LR"(\s*\"(((\\\")|[^\"])*)\"\s*)"};
const std::wregex spaces_regex{LR"(\s*(.*\S)\s*)"};
const std::wregex array_regex{LR"(\[([^\]]*)\])"};
};

template <typename CharT>
struct array_whitespace{};

template <>
struct array_whitespace<char> : std::ctype<char>
{
    array_whitespace() : std::ctype<char>(get_table()) {}

    static std::ctype_base::mask const* get_table()
    {
        static std::vector<std::ctype_base::mask>
                rc(table_size, std::ctype_base::mask());
        rc['\n'] = std::ctype_base::space;
        rc[','] = std::ctype_base::space;
        rc[' '] = ~std::ctype_base::space;

        return rc.data();
    }
};

template <>
struct array_whitespace<wchar_t> : std::ctype<wchar_t>
{
    bool do_is(mask m, char_type c) const override
    {
        if((m & space) && c == L' ')
            return false;
        if((m & space) && c == L',')
            return true;

        return ctype::do_is(m, c);
    }
};

}

}

#endif //INI_SYNAX_H
