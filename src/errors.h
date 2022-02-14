#ifndef INI_ERRORS_H
#define INI_ERRORS_H

#include <string>
#include <exception>

namespace ini
{

class not_convertable : public std::exception
{
public:
    const char* what() const noexcept override
    {
        return "Could not convert to type";
    }
};

class parsing_error : public std::exception
{
public:
    explicit parsing_error(size_t line_no) noexcept
        : m_line_no(line_no)
    {
        m_mes = "Error in line " + std::to_string(line_no) + ": ";
    }

    const char *what() const noexcept override
    {
        return m_mes.c_str();
    }

    size_t getLineNumber() const { return m_line_no; }
protected:
    std::string m_mes;
    size_t m_line_no;
};

class parsing_fail : public parsing_error
{
public:
    explicit parsing_fail(size_t line_no, const std::string& line) noexcept
        : parsing_error(line_no)
    {
        m_mes += "failed to parse line '" + line + "'";
    }
};

class out_of_section_declaration : public parsing_error
{
public:
    explicit out_of_section_declaration(size_t line_no) noexcept
        : parsing_error(line_no)
    {
        m_mes += "out of section declaration";
    }
};

class section_error : public parsing_error
{
public:
    explicit section_error(size_t line_no, const std::string& section_name) noexcept
        : parsing_error(line_no), m_section_name(section_name) {}
protected:
    std::string m_section_name;
};

class double_section_definition : public section_error
{
public:
    explicit double_section_definition(size_t line_no, const std::string& section_name) noexcept
        : section_error(line_no, section_name)
    {
        m_mes += "double definition of section '" + m_section_name + "'";
    }
};

class value_error : public section_error
{
public:
    explicit value_error(size_t line_no, const std::string& section_name, const std::string& value_name) noexcept
        : section_error(line_no, section_name), m_value_name(value_name) {}
protected:
    std::string m_value_name;
};

class double_value_definition : public value_error
{
public:
    explicit double_value_definition(size_t line_no, const std::string& section_name, const std::string& value_name) noexcept
        : value_error(line_no, section_name, value_name)
    {
        m_mes += "double definition of value '" + m_value_name + "' in section '" + m_section_name + "'";
    }
};

}

#endif //INI_ERRORS_H
