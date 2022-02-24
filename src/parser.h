#ifndef INI_PARSER_H
#define INI_PARSER_H

#include <map>
#include <string>
#include <regex>
#include <utility>
#include <fstream>
#include "value.h"
#include "errors.h"

namespace ini
{

template <typename String>
class Line;

template <typename String>
class Section;

template <typename String>
class File;

template <typename Iter, typename String>
void parse(Iter begin_iter, Iter end_iter, File<String>& file);

template <typename String>
void parse(const std::string& filename, File<String>& file);

template <typename CharT, typename Traits, typename Allocator>
class Line<std::basic_string<CharT, Traits, Allocator>> : public std::basic_string<CharT, Traits, Allocator>
{
public:
    friend std::istream& operator>>(std::basic_istream<CharT, Traits>& is, Line& line)
    {
        using std::getline;
        return getline(is, line);
    }
};

namespace details
{

template <typename V>
struct map_derived_helper
{
    using type = void;
};

template <template <class> class V, typename CharT, typename Traits, typename Allocator>
struct map_derived_helper<V<std::basic_string<CharT, Traits, Allocator> > >
{
    using string_type = std::basic_string<CharT, Traits, Allocator>;
    using type = std::map<string_type, V<string_type>, std::less<string_type>, Allocator>;
};

template <typename V>
using map_derived_helper_t = typename map_derived_helper<V>::type;

template <typename V>
struct map_derived : private map_derived_helper_t<V>
{
    using string_type = typename map_derived_helper_t<V>::key_type;
    using allocator_type = typename map_derived_helper_t<V>::allocator_type;

    using map_derived_helper_t<V>::map_derived_helper_t;
    using map_derived_helper_t<V>::at;
    using map_derived_helper_t<V>::begin;
    using map_derived_helper_t<V>::end;
    using map_derived_helper_t<V>::find;
    using map_derived_helper_t<V>::clear;

protected:
    using map_derived_helper_t<V>::get_allocator;
    using map_derived_helper_t<V>::emplace;
};

}

template <typename S>
class Section : public details::map_derived<BasicValue<S>>
{
public:
    using string_type = typename details::map_derived<BasicValue<S>>::string_type;
    using allocator_type = typename details::map_derived<BasicValue<S>>::allocator_type;

    inline explicit Section(string_type section_name);

    template <typename T>
    T get(const string_type& name, const T& default_value = T()) const;

    template <typename Iter, typename String>
    friend void parse(Iter begin_iter, Iter end_iter, File<String>& file);

private:
    inline void addFromString(size_t line_no, const string_type& str);

    const string_type m_section_name;
};

template <typename S>
class File : public details::map_derived<Section<S>>
{
public:
    using string_type = typename details::map_derived<Section<S>>::string_type;
    using allocator_type = typename details::map_derived<Section<S>>::allocator_type;

    explicit File(allocator_type alloc = allocator_type()) : details::map_derived<Section<S>>(alloc) {}

    template <typename Iter, typename String>
    friend void parse(Iter begin_iter, Iter end_iter, File<String>& file);
};

template <typename S>
Section<S>::Section(string_type section_name)
        : details::map_derived<BasicValue<S>>(section_name.get_allocator()), m_section_name(std::move(section_name)) {}

template <typename S>
template <typename T>
T Section<S>::get(const string_type& name, const T& default_value) const
{
    if(this->find(name) != this->end())
        return this->at(name).template as<T>();
    return default_value;
}

template <typename S>
void Section<S>::addFromString(size_t line_no, const string_type& str)
{
    std::match_results<typename string_type::const_iterator, allocator_type> match(this->get_allocator());
    if(!std::regex_match(str, match, syntax::ini_traits<typename string_type::value_type>::value_regex()))
        throw parsing_fail(line_no, str);

    if(this->find(match[1].str()) != this->end())
        throw double_value_definition(line_no, m_section_name, match[1].str());

    this->emplace(std::piecewise_construct, std::forward_as_tuple(match[1].str()),
                   std::forward_as_tuple(match[3].str()));
}

template <typename Iter, typename String>
void parse(Iter begin_iter, Iter end_iter, File<String>& file)
{
    using char_type = typename String::value_type;
    using ini_traits = syntax::ini_traits<char_type>;

    file.clear();
    String current_section(file.get_allocator());
    std::match_results<typename String::const_iterator, typename String::allocator_type> match(file.get_allocator());
    size_t line_no = 1;
    for(Iter it = begin_iter; it != end_iter; ++it, ++line_no)
    {
        if(it->empty() || std::regex_match(*it, ini_traits::comment_line_regex()))
            continue;
        if(std::regex_match(*it, match, ini_traits::section_name_regex()))
        {
            current_section = match[1].str();
            if(file.find(current_section) != file.end())
                throw double_section_definition(line_no, current_section);
            file.emplace(std::piecewise_construct, std::forward_as_tuple(current_section),
                                std::forward_as_tuple(current_section));
        }
        else
        {
            if(current_section.empty())
                throw out_of_section_declaration(line_no);
            file.at(current_section).addFromString(line_no, *it);
        }
    }
}

template <typename String>
void parse(const std::string& filename, File<String>& file)
{
    std::ifstream ifs(filename);
    parse(std::istream_iterator<Line<String>>(ifs), std::istream_iterator<Line<String>>(), file);
}

}

#endif //INI_PARSER_H
