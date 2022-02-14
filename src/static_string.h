#ifndef INI_STATIC_STRING_H
#define INI_STATIC_STRING_H

#include <array>

namespace ini
{

namespace str
{

template <size_t N>
class static_string
{
public:
    constexpr explicit static_string(const char (&str)[N]) noexcept
        : static_string(str, std::make_index_sequence<N>{}) {}

    constexpr const char* data() const noexcept { return m_data.data(); }
    constexpr size_t size() const noexcept { return N; }
private:
    template <size_t... I>
    constexpr static_string(const char (&str)[N], std::index_sequence<I...>) noexcept
        : m_data{str[I]..., '\0'} {}
    
    std::array<char, N + 1> m_data;
};

template <size_t N>
constexpr static_string<N> make_static_string(const char (&str)[N]) noexcept
{
    return static_string<N>{str};
}

}

}

#endif //INI_STATIC_STRING_H
