#ifndef INI_PARSER_VALUE_H
#define INI_PARSER_VALUE_H

#include <string>
#include <sstream>
#include <regex>
#include "errors.h"
#include "synax.h"

namespace ini
{

/**
 * Type tag
 * @tparam T type
 */
template <typename T>
struct tag_t {};

template <typename String>
class BasicValue;
//{
//public:
//    BasicValue() = delete;
//    BasicValue(const BasicValue<String>&) = delete;
//    BasicValue(BasicValue<String>&&) = delete;
//};

/**
 * Value class for containing any from string convertible values
 */
template <typename CharT, typename Traits, typename Allocator>
class BasicValue<std::basic_string<CharT, Traits, Allocator>>
{
public:
    using string_type = std::basic_string<CharT, Traits, Allocator>;
    /**
     * @brief Default constructor
     * @attention will contain an empty value only
     */
    inline BasicValue(Allocator alloc = Allocator());
    /**
     * @brief Copy string constructor
     * @param str string containing a value
     */
    inline explicit BasicValue(const string_type& str);
    /**
     * @brief Move string constructor
     * @param str string containing a value
     */
    inline explicit BasicValue(string_type&& str);

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

    string_type m_str_value;
};

typedef BasicValue<std::string> Value;
typedef BasicValue<std::wstring> wValue;

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

template <typename CharT, typename Traits, typename Allocator>
typename std::basic_string<CharT, Traits, Allocator> from_string(
        tag_t<std::basic_string<CharT, Traits, Allocator>>,
        const std::basic_string<CharT, Traits, Allocator>& str)
{
    using string_type = std::basic_string<CharT, Traits, Allocator>;

    std::match_results<typename string_type::const_iterator, Allocator> match;
    string_type res;
    if(!std::regex_match(str, match, syntax::value_traits<CharT>::comma_regex()))
        std::regex_match(str, match, syntax::value_traits<CharT>::spaces_regex());
    res = match[1].str();

    auto slash_end = std::remove(res.begin(), res.end(), '\\');
    res.erase(slash_end, res.end());
    return res;
}

template <typename CharT, typename Traits, typename Allocator>
BasicValue<std::basic_string<CharT, Traits, Allocator>> from_string(tag_t<BasicValue<std::basic_string<CharT, Traits, Allocator>>>,
                                                 const std::basic_string<CharT, Traits, Allocator>& str) noexcept
{
    return BasicValue<std::basic_string<CharT, Traits, Allocator>>(str);
}

template <typename CharT, typename Traits, typename Allocator, typename T>
std::enable_if_t<std::is_fundamental<T>::value, T> from_string(tag_t<T>,
        const std::basic_string<CharT, Traits, Allocator>& str)
{
    std::basic_istringstream<CharT, Traits, Allocator> iss(str);
    T res;
    iss >> res;
    if(iss.fail())
        throw not_convertible();
    return res;
}

template <typename CharT, typename Traits, typename Allocator, typename T,
        typename = decltype(operator>>(std::declval<std::basic_istream<CharT, Traits>&>(), std::declval<T&>()))>
auto from_string(tag_t<T>, const std::basic_string<CharT, Traits, Allocator>& str) -> decltype(T())
{
    std::basic_istringstream<CharT, Traits, Allocator> iss(str);
    T res;
    iss >> res;
    if(iss.fail())
        throw not_convertible();
    return res;
}

template <typename CharT, typename Traits, typename Allocator, typename T>
auto from_string(tag_t<T>, const std::basic_string<CharT, Traits, Allocator>& str)
    noexcept(noexcept(T::from_string(str)))
    -> decltype(T::from_string(str))
{
    return T::from_string(str);
}

template <typename CharT, typename Traits, typename Allocator, typename T>
auto from_string(tag_t<T>, const std::basic_string<CharT, Traits, Allocator>& str)
    noexcept(noexcept(customization::stringer<T>::_(str)))
    -> decltype(customization::stringer<T>::_(str))
{
    return customization::stringer<T>::_(str);
}

template <typename CharT, typename Traits, typename Allocator, typename T,
        typename = decltype(
                T().push_back(from_string(ini::tag_t<typename T::value_type>(),
                        std::declval<std::basic_string<CharT, Traits, Allocator>>()))
                )>
T from_string(tag_t<T>, const std::basic_string<CharT, Traits, Allocator>& str)
{
    using value_t = typename T::value_type;
    using string_type = std::basic_string<CharT, Traits, Allocator>;

    std::match_results<typename string_type::const_iterator, Allocator> match;
    if(!std::regex_match(str, match, syntax::value_traits<CharT>::array_regex()))
        throw not_convertible();
    std::basic_istringstream<CharT, Traits, Allocator> iss(match[1].str());
    iss.imbue(std::locale(iss.getloc(), syntax::value_traits<CharT>::array_ctype()));
    T res;
    std::transform(std::istream_iterator<string_type, CharT, Traits>(iss),
                   std::istream_iterator<string_type, CharT, Traits>(),
                   std::back_inserter(res), [](const string_type& str) { return from_string(tag_t<value_t>(), str); });
    return res;
}

struct from_string_fn
{
    template <typename CharT, typename Traits, typename Allocator, typename T>
    auto operator()(tag_t<T>, const std::basic_string<CharT, Traits, Allocator>& str) const
        -> decltype(from_string(tag_t<T>(), str))
    {
        return from_string(tag_t<T>(), str);
    }
};

}

namespace
{

constexpr details::from_string_fn from_string;

}

template <typename CharT, typename Traits, typename Allocator>
BasicValue<std::basic_string<CharT, Traits, Allocator>>::BasicValue(Allocator alloc)
    : m_str_value(alloc) {}

template <typename CharT, typename Traits, typename Allocator>
BasicValue<std::basic_string<CharT, Traits, Allocator>>::BasicValue(const string_type& str)
        : m_str_value(str) {}

template <typename CharT, typename Traits, typename Allocator>
BasicValue<std::basic_string<CharT, Traits, Allocator>>::BasicValue(string_type&& str)
        : m_str_value(std::move(str)) {}

template <typename CharT, typename Traits, typename Allocator>
template <typename T>
T BasicValue<std::basic_string<CharT, Traits, Allocator>>::as(const T& default_value) const
{
    if(empty())
        return default_value;
    return from_string(tag_t<T>(), m_str_value);
}

template <typename CharT, typename Traits, typename Allocator>
template <typename T>
T BasicValue<std::basic_string<CharT, Traits, Allocator>>::as() const
{
    if(empty())
        return get_default<T>(std::is_default_constructible<T>());
    return from_string(tag_t<T>(), m_str_value);
}

}

#endif //INI_PARSER_VALUE_H
