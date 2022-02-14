#ifndef INI_CONST_STRING_MAP_H
#define INI_CONST_STRING_MAP_H

#include <map>
#include <string>
#include <iterator>

namespace ini
{

template <typename T>
class const_string_map
{
public:
    using value_type = T;
    using iterator = typename std::map<std::string, value_type>::const_iterator;

    bool contain(const std::string& name) const
    {
        return (m_data.find(name) != m_data.end());
    }

    const value_type& at(const std::string& name) const
    {
        if(!contain(name))
            throw std::range_error("No value '" + name + "'");
        return m_data.at(name);
    }

    iterator begin() const { return m_data.begin(); }
    iterator end() const { return m_data.end(); }
protected:
    const_string_map() = default;

    std::map<std::string, value_type> m_data;
};

}

#endif //INI_CONST_STRING_MAP_H
