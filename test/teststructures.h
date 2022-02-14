#ifndef INI_TESTSTRUCTURES_H
#define INI_TESTSTRUCTURES_H

#include <string>
#include "value.h"

struct A
{
    A(int v) : val(v) {}
    int val;

    static A from_string(const std::string& str)
    {
        return A(std::stoi(str));
    }
};

struct B
{
    B() = default;
    B(int v) : val(v) {}
    int val;
};

inline std::istream& operator>>(std::istream& is, B& b)
{
    is >> b.val;
    return is;
}

namespace user
{

struct C
{
    C(int v) : val(v) {}
    int val;
};

inline C from_string(ini::tag_t<C>, const std::string& str)
{
    return C(std::stoi(str));
}

}

namespace other
{

struct D
{
    D(int v) : val(v) {}
    int val;
};

}

namespace ini::customization
{

template <>
struct stringer<other::D>
{
    static other::D _(const std::string& str)
    {
        return other::D(std::stoi(str));
    }
};

}

namespace user
{

enum class test_enum{ one, two, three, four };

inline bool operator==(test_enum l, test_enum r)
{
    return static_cast<int>(l) == static_cast<int>(r);
}

inline test_enum from_string(ini::tag_t<test_enum>, const std::string& str)
{
    if(str == "test_enum::one")
        return test_enum::one;
    else if(str == "test_enum::two")
        return test_enum::two;
    else if(str == "test_enum::three")
        return test_enum::three;
    else if(str == "test_enum::four")
        return test_enum::four;
    else
        throw ini::not_convertable();
}

}

#endif //INI_TESTSTRUCTURES_H
