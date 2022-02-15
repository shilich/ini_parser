#ifndef INI_PARSER_H
#define INI_PARSER_H

#include <map>
#include <string>
#include <regex>
#include <utility>
#include <fstream>
#include "value.h"
#include "errors.h"
#include "const_string_map.h"

namespace ini
{

class Line : public std::string
{
public:
    friend std::istream& operator>>(std::istream& is, Line& line)
    {
        return std::getline(is, line);
    }
};

class File;

template <typename Iter>
void parse(Iter begin_iter, Iter end_iter, File& file);

inline void parse(const std::string& filename, File& file);

class Section : public const_string_map<Value>
{
public:
    inline explicit Section(std::string section_name);

    inline const std::string& getName() const { return m_section_name; }

    template <typename T>
    T get(const std::string& name, const T& default_value = T()) const;

    template <typename Iter>
    friend void parse(Iter begin_iter, Iter end_iter, File& file);

private:
    inline void addFromString(size_t line_no, const std::string& str);

    const std::string m_section_name;
};

class File : public const_string_map<Section>
{
public:
    File() = default;

    template <typename Iter>
    friend void parse(Iter begin_iter, Iter end_iter, File& file);
};

Section::Section(std::string  section_name)
        : m_section_name(std::move(section_name)){}

template <typename T>
T Section::get(const std::string& name, const T& default_value) const
{
    if(contain(name))
        return m_data.at(name).as<T>();
    return default_value;
}

void Section::addFromString(size_t line_no, const std::string& str)
{
    static std::regex value_regex(R"(^\s*([\w_][\w\d_]*)\s*(:=|:|=)\s*([^;]+[^;\s])\s*(;.*)?$)");
    std::smatch match;
    if(!std::regex_match(str, match, value_regex))
        throw parsing_fail(line_no, str);

    if(contain(match[1].str()))
        throw double_value_definition(line_no, m_section_name, match[1].str());

    m_data.emplace(std::piecewise_construct, std::forward_as_tuple(match[1].str()),
                   std::forward_as_tuple(match[3].str()));
}

template <typename Iter>
void parse(Iter begin_iter, Iter end_iter, File& file)
{
    static std::regex section_name_regex(R"(^\s*\[\s*([\w_][\w\d_]*)\s*\]$)");
    static std::regex comment_line_regex(R"(^\s*;.*$)");

    file.m_data.clear();
    std::string current_section;
    std::smatch match;
    size_t line_no = 1;
    for(Iter it = begin_iter; it != end_iter; ++it, ++line_no)
    {
        if(it->empty() || std::regex_match(*it, comment_line_regex))
            continue;
        if(std::regex_match(*it, match, section_name_regex))
        {
            current_section = match[1].str();
            if(file.contain(current_section))
                throw double_section_definition(line_no, current_section);
            file.m_data.emplace(std::piecewise_construct, std::forward_as_tuple(current_section),
                                std::forward_as_tuple(current_section));
        }
        else
        {
            if(current_section.empty())
                throw out_of_section_declaration(line_no);
            file.m_data.at(current_section).addFromString(line_no, *it);
        }
    }
}

void parse(const std::string& filename, File& file)
{
    std::ifstream ifs(filename);
    parse(std::istream_iterator<Line>(ifs), std::istream_iterator<Line>(), file);
}

}

#endif //INI_PARSER_H
