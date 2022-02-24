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

template <typename CharT, typename Traits, typename Allocator>
class Section<std::basic_string<CharT, Traits, Allocator>>
    : private std::map<
                    std::basic_string<CharT, Traits, Allocator>,
                    BasicValue<std::basic_string<CharT, Traits, Allocator>>,
                    std::less<std::basic_string<CharT, Traits, Allocator>>,
                    Allocator
                    >
{
public:
    using base = std::map<
                        std::basic_string<CharT, Traits, Allocator>,
                        BasicValue<std::basic_string<CharT, Traits, Allocator>>,
                        std::less<std::basic_string<CharT, Traits, Allocator>>,
                        Allocator
                        >;
    using string_type = std::basic_string<CharT, Traits, Allocator>;

    using base::at;
    using base::begin;
    using base::end;
    using base::find;

    inline explicit Section(string_type section_name);

    inline const string_type& getName() const { return m_section_name; }

    template <typename T>
    T get(const string_type& name, const T& default_value = T()) const;

    template <typename Iter, typename String>
    friend void parse(Iter begin_iter, Iter end_iter, File<String>& file);

private:
    inline void addFromString(size_t line_no, const string_type& str);

    const string_type m_section_name;
};

template <typename CharT, typename Traits, typename Allocator>
class File<std::basic_string<CharT, Traits, Allocator>>
    : private std::map<
                        std::basic_string<CharT, Traits, Allocator>,
                        Section<std::basic_string<CharT, Traits, Allocator>>,
                        std::less<std::basic_string<CharT, Traits, Allocator>>,
                        Allocator
                        >
{
public:
    using base = std::map<
            std::basic_string<CharT, Traits, Allocator>,
            Section<std::basic_string<CharT, Traits, Allocator>>,
            std::less<std::basic_string<CharT, Traits, Allocator>>,
            Allocator
    >;
    using string_type = std::basic_string<CharT, Traits, Allocator>;

    using base::at;
    using base::begin;
    using base::end;
    using base::find;

    explicit File(Allocator alloc = Allocator()) : base(alloc) {}

    template <typename Iter, typename String>
    friend void parse(Iter begin_iter, Iter end_iter, File<String>& file);

private:
    using base::clear;
};

template <typename CharT, typename Traits, typename Allocator>
Section<std::basic_string<CharT, Traits, Allocator>>::Section(string_type section_name)
        : base(section_name.get_allocator()), m_section_name(std::move(section_name)) {}

template <typename CharT, typename Traits, typename Allocator>
template <typename T>
T Section<std::basic_string<CharT, Traits, Allocator>>::get(const string_type& name, const T& default_value) const
{
    if(this->find(name) != this->end())
        return at(name).template as<T>();
    return default_value;
}

template <typename CharT, typename Traits, typename Allocator>
void Section<std::basic_string<CharT, Traits, Allocator>>::addFromString(size_t line_no, const string_type& str)
{
    std::match_results<typename string_type::const_iterator, Allocator> match(this->get_allocator());
    if(!std::regex_match(str, match, syntax::ini_traits<CharT>::value_regex()))
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
