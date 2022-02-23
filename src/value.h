#ifndef INI_PARSER_VALUE_H
#define INI_PARSER_VALUE_H

#include <string>
#include <sstream>
#include <regex>
#include "errors.h"

namespace ini
{

/**
 * Type tag
 * @tparam T type
 */
template <typename T>
struct tag_t {};

/**
 * Value class for containing any from string convertible values
 */
class Value
{
public:
    /**
     * @brief Default constructor
     * @attention will contain an empty value only
     */
    inline Value() = default;
    /**
     * @brief Copy string constructor
     * @param str string containing a value
     */
    inline explicit Value(const std::string& str);
    /**
     * @brief Move string constructor
     * @param str string containing a value
     */
    inline explicit Value(std::string&& str);

    /**
     * @brief Convert to type
     * @tparam T type to convert to
     * @return value of type T containing in value string or T() if string was empty
     * @throw ini::not_convertible if convert wasn't success
     * @throw std::invalid_argument if value strung was empty and T is not default constructible
     */
    template <typename T>
    T as() const;

    /**
     * @brief Convert to type
     * @tparam T type to convert to
     * @param default_value value to return if value string is empty
     * @return value of type T containing in value string or default_value if string was empty
     * @throw ini::not_convertible if convert wasn't success
     */
    template <typename T>
    T as(const T& default_value) const;

    //! Returns true if value is empty
    inline bool empty() const { return m_str_value.empty(); }
private:
    template <typename T>
    static T get_default(std::true_type) { return T(); }

    template <typename T>
    static T get_default(std::false_type) { throw std::invalid_argument("No default value!"); }

    std::string m_str_value;
};

/**
 * Customization namespace
 * Use to define from string converts of types you haven't got access to
 * Possible ways to customize:
 *
 * 1) Define static method from_string in your classes
 * @code
 * class my_class
 * {
 *  ...
 * public:
 *      static my_class from_string(const std::string& str)
 *      {
 *          ...
 *      }
 * };
 * @endcode
 * 2) Define operator>> in your namespaces
 * @code
 * namespace user
 * {
 *
 * class my_class { ... };
 *
 * std::istream& operator>>(std::istream& is, my_class& value)
 * {
 *      ...
 * }
 *
 * }
 * @endcode
 * 3) Define from_string(ini::tag_t, std::string) in your namespaces
 * @code
 * namespace user
 * {
 *
 * class my_class { ... };
 *
 * my_class from_string(ini::tag_t<my_class>, const std::string& str)
 * {
 *      ...
 * }
 *
 * }
 * @endcode
 * 4) Specialize ini::customization::stringer struct
 * @see stringer
 */
namespace customization
{

/**
 * @struct stringer
 * Structure to define string converting of type
 * @tparam T type to convert
 * Specialize this template structure with static void _(const std::string&) for your types
 * @example
 * @code
 * namespace ini::customization
 * {
 *
 * template <>
 * struct stringer<my_type>
 * {
 *      static my_type _(const std::string& str)
 *      {
 *          ...
 *      }
 * }
 *
 * }
 * @endcode
 */
template <typename T>
struct stringer {};

}

namespace details
{

struct array_whitespace : std::ctype<char>
{
    array_whitespace() : std::ctype<char>(get_table()) {}

    static std::ctype_base::mask const* get_table()
    {
        static std::vector<std::ctype_base::mask>
                rc(table_size, std::ctype_base::mask());
        rc['\n'] = std::ctype_base::space;
        rc[','] = std::ctype_base::space;
        rc[' '] = std::ctype_base::punct;

        return rc.data();
    }
};

inline std::string from_string(tag_t<std::string>, const std::string& str)
{
    static std::regex comma_regex(R"(\s*\"(((\\\")|[^\"])*)\"\s*)");
    static std::regex spaces_regex(R"(\s*(.*\S)\s*)");
    std::smatch match;
    std::string res;
    if(!std::regex_match(str, match, comma_regex))
        std::regex_match(str, match, spaces_regex);
    res = match[1].str();

    auto slash_end = std::remove(res.begin(), res.end(), '\\');
    res.erase(slash_end, res.end());
    return res;
}

inline Value from_string(tag_t<Value>, const std::string& str)
{
    return Value(str);
}

template <typename T>
std::enable_if_t<std::is_fundamental<T>::value, T> from_string(tag_t<T>, const std::string& str)
{
    std::istringstream iss(str);
    T res;
    iss >> res;
    if(iss.fail())
        throw not_convertible();
    return res;
}

template <typename T, typename = decltype(operator>>(std::declval<std::istream&>(), std::declval<T&>()))>
auto from_string(tag_t<T>, const std::string& str) -> decltype(T())
{
    std::istringstream iss(str);
    T res;
    iss >> res;
    if(iss.fail())
        throw not_convertible();
    return res;
}

template <typename T>
auto from_string(tag_t<T>, const std::string& str)
    -> decltype(T::from_string(str))
{
    return T::from_string(str);
}

template <typename T>
auto from_string(tag_t<T>, const std::string& str)
    -> decltype(customization::stringer<T>::_(str))
{
    return customization::stringer<T>::_(str);
}

template <typename T, typename = decltype(T().push_back(from_string(ini::tag_t<typename T::value_type>(), std::declval<std::string>())))>
T from_string(tag_t<T>, const std::string& str)
{
    using value_t = typename T::value_type;
    static std::regex array_regex(R"(\[([^\]]*)\])");
    std::smatch match;
    if(!std::regex_match(str, match, array_regex))
        throw not_convertible();
    std::istringstream iss(match[1].str());
    iss.imbue(std::locale(iss.getloc(), new array_whitespace));
    T res;
    auto inserter = std::back_inserter(res);
    std::string tmp;
    while(iss >> tmp)
        inserter = from_string(tag_t<value_t>(), tmp);
    return res;
}

struct from_string_fn
{
    template <typename T>
    auto operator()(tag_t<T>, const std::string& str) const -> decltype(from_string(tag_t<T>(), str))
    {
        return from_string(tag_t<T>(), str);
    }
};

}

namespace
{

constexpr details::from_string_fn from_string;

}

Value::Value(const std::string& str)
        : m_str_value(str) {}

Value::Value(std::string&& str)
        : m_str_value(std::move(str)) {}

template <typename T>
T Value::as(const T& default_value) const
{
    if(empty())
        return default_value;
    return from_string(tag_t<T>(), m_str_value);
}

template <typename T>
T Value::as() const
{
    if(empty())
        return get_default<T>(std::is_default_constructible<T>());
    return from_string(tag_t<T>(), m_str_value);
}

}

#endif //INI_PARSER_VALUE_H
