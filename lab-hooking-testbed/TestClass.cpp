#include "TestClass.h"
#include <iostream>

int TestClass::Instance_SomeFunc()
{
    std::cout << "*** TestClass::Instance_SomeFunc executed ***\n"
        "\targs[0]: implicit this = " << std::hex << reinterpret_cast<uintptr_t>(this) << "\n"
        "\t\tthis->SomeIntField: " << this->SomeIntField << "\n"
        "\t\tthis->SomeFloatField: " << this->SomeFloatField << "\n";

    return 1;
}

int TestClass::Virtual_SomeFunc(int derp)
{
    std::cout << "*** TestClass::Virtual_SomeFunc executed ***\n"
        "\targs[0]: implicit this = " << std::hex << reinterpret_cast<uintptr_t>(this) << "\n"
        "\t\tthis->SomeIntField: " << this->SomeIntField << "\n"
        "\t\tthis->SomeFloatField: " << this->SomeFloatField << "\n"
        "\targs[1]: derp = " << derp << "\n";

    return 1;
}

void TestClass::Virtual_SomeOtherFunc()
{
    std::cout << "*** TestClass::Virtual_SomeOtherFunc executed ***\n"
        "\targs[0]: implicit this = " << std::hex << reinterpret_cast<uintptr_t>(this) << "\n"
        "\t\tthis->SomeIntField: " << this->SomeIntField << "\n"
        "\t\tthis->SomeFloatField: " << this->SomeFloatField << "\n";
}

bool TestClass::Virtual_LastVirtFunc(const char* str)
{
    std::cout << "*** TestClass::Virtual_LastVirtFunc executed ***\n"
        "\targs[0]: implicit this = " << std::hex << reinterpret_cast<uintptr_t>(this) << "\n"
        "\t\tthis->SomeIntField: " << this->SomeIntField << "\n"
        "\t\tthis->SomeFloatField: " << this->SomeFloatField << "\n"
        "\targs[1]: str = " << str << "\n";

    return true;
}