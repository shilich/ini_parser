#ifndef INI_SYNAX_H
#define INI_SYNAX_H

#include <regex>

namespace ini
{

namespace syntax
{

template <typename CharT, typename Traits = std::regex_traits<CharT>>
struct ini_traits
{
    using regex_type = std::basic_regex<CharT, Traits>;
    static regex_type& value_regex();
    static regex_type& section_name_regex();
    static regex_type& comment_line_regex();
};

template <typename CharT, typename Traits = std::regex_traits<CharT>>
struct value_traits
{
    using regex_type = std::basic_regex<CharT, Traits>;
    static regex_type& comma_regex();
    static regex_type& spaces_regex();
    static regex_type& array_regex();

    static std::ctype<CharT>* array_ctype();
};

template <>
inline typename ini_traits<char>::regex_type& ini_traits<char>::value_regex()
{
    static ini_traits<char>::regex_type res(R"(^\s*([\w_][\w\d_]*)\s*(:=|:|=)\s*([^;]+[^;\s])\s*(;.*)?$)");
    return res;
}

template <>
inline typename ini_traits<char>::regex_type& ini_traits<char>::section_name_regex()
{
    static ini_traits<char>::regex_type res(R"(^\s*\[\s*([\w_][\w\d_]*)\s*\]$)");
    return res;
}

template <>
inline typename ini_traits<char>::regex_type& ini_traits<char>::comment_line_regex()
{
    static ini_traits<char>::regex_type res(R"(^\s*;.*$)");
    return res;
}

template <>
inline typename ini_traits<wchar_t>::regex_type& ini_traits<wchar_t>::value_regex()
{
    static ini_traits<wchar_t>::regex_type res(LR"(^\s*([\w_][\w\d_]*)\s*(:=|:|=)\s*([^;]+[^;\s])\s*(;.*)?$)");
    return res;
}

template <>
inline typename ini_traits<wchar_t>::regex_type& ini_traits<wchar_t>::section_name_regex()
{
    static ini_traits<wchar_t>::regex_type res(LR"(^\s*\[\s*([\w_][\w\d_]*)\s*\]$)");
    return res;
}

template <>
inline typename ini_traits<wchar_t>::regex_type& ini_traits<wchar_t>::comment_line_regex()
{
    static ini_traits<wchar_t>::regex_type res(LR"(^\s*;.*$)");
    return res;
}

template <>
inline typename value_traits<char>::regex_type& value_traits<char>::comma_regex()
{
    static value_traits<char>::regex_type res(R"(\s*\"(((\\\")|[^\"])*)\"\s*)");
    return res;
}

template <>
inline typename value_traits<char>::regex_type& value_traits<char>::spaces_regex()
{
    static value_traits<char>::regex_type res(R"(\s*(.*\S)\s*)");
    return res;
}

template <>
inline typename value_traits<char>::regex_type& value_traits<char>::array_regex()
{
    static value_traits<char>::regex_type res(R"(\[([^\]]*)\])");
    return res;
}

template <>
inline typename value_traits<wchar_t>::regex_type& value_traits<wchar_t>::comma_regex()
{
	static value_traits<wchar_t>::regex_type res(LR"(\s*\"(((\\\")|[^\"])*)\"\s*)");
	return res;
}

template <>
inline typename value_traits<wchar_t>::regex_type& value_traits<wchar_t>::spaces_regex()
{
	static value_traits<wchar_t>::regex_type res(LR"(\s*(.*\S)\s*)");
	return res;
}

template <>
inline typename value_traits<wchar_t>::regex_type& value_traits<wchar_t>::array_regex()
{
	static value_traits<wchar_t>::regex_type res(LR"(\[([^\]]*)\])");
	return res;
}

namespace details
{

template <typename CharT>
struct array_whitespace_ctype{};

template <>
struct array_whitespace_ctype<char> : std::ctype<char>
{
    array_whitespace_ctype() : std::ctype<char>(get_table()){}

    static std::ctype_base::mask const *get_table()
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
struct array_whitespace_ctype<wchar_t> : std::ctype<wchar_t>
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

template <>
inline std::ctype<char>* value_traits<char>::array_ctype()
{
    return new details::array_whitespace_ctype<char>;
}

template <>
inline std::ctype<wchar_t>* value_traits<wchar_t>::array_ctype()
{
    return new details::array_whitespace_ctype<wchar_t>;
}

}

}

#endif //INI_SYNAX_H
