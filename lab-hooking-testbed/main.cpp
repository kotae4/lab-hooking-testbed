#include "TestClass.h"
#pragma region BoilerplateGarbage

#include <polyhook2/CapstoneDisassembler.hpp>
#ifdef _M_IX86
#include <polyhook2/Detour/x86Detour.hpp>
#else
#include <polyhook2/Detour/x64Detour.hpp>
#endif
#include <polyhook2/Exceptions/HWBreakPointHook.hpp>

#include <MinHook.h>
template <typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
{
    return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}
#include <iostream>
#include <string>
#include <sstream>

void InstallAllHooks(TestClass* pTestInst);

typedef int(__fastcall* dTestClass_InstanceSomeFunc)(TestClass* that);
dTestClass_InstanceSomeFunc oInstanceSomeFunc = NULL;
int hk_TestClass_InstanceSomeFunc(TestClass* that);

typedef int(__fastcall* dTestClass_VirtualSomeFunc)(TestClass* that, int derp);
dTestClass_VirtualSomeFunc oVirtualSomeFunc = NULL;
#ifdef _M_IX86
PLH::x86Detour* phDetour_VirtualSomeFunc = NULL;
#else
PLH::x64Detour* phDetour_VirtualSomeFunc = NULL;
#endif
int hk_TestClass_VirtualSomeFunc(TestClass* that, int derp);

typedef void(__fastcall* dTestClass_VirtualSomeOtherFunc)(TestClass* that);
dTestClass_VirtualSomeOtherFunc oVirtualSomeOtherFunc = NULL;
void hk_TestClass_VirtualSomeOtherFunc(TestClass* that);

typedef bool(__fastcall* dTestClass_LastVirtFunc)(TestClass* that, const char* str);
dTestClass_LastVirtFunc oLastVirtFunc = NULL;
bool hk_TestClass_LastVirtFunc(TestClass* that, const char* str);

typedef int(*dSomeGlobalFunc)(double derp);
dSomeGlobalFunc oSomeGlobalFunc = NULL;
PLH::HWBreakPointHook* phHWBP_SomeGlobalFunc = NULL;
int hk_SomeGlobalFunc(double derp);

template<typename TOUT, typename TIN>
TOUT ForceCast(TIN in)
{
    union
    {
        TIN  in;
        TOUT out;
    }
    u = { in };

    return u.out;
};

#pragma endregion

int SomeGlobalFunc(double derp)
{
    std::cout << "*** SomeGlobalFunc executed ***\n"
        "\targs[0]: derp: " << derp << "\n";

    return 1;
}

int main()
{
    TestClass* testInst = new TestClass;
    testInst->SomeIntField = 20;
    testInst->SomeFloatField = 1.5f;

    std::cout << "testInst is located @ " << std::hex << reinterpret_cast<uintptr_t>(testInst) << " and every 'implicit this' should match\n";
    
    InstallAllHooks(testInst);

    std::cout << "Calling testInst->Instance_SomeFunc now\n";
    testInst->Instance_SomeFunc();
    std::cout << "Calling testInst->Virtual_SomeFunc now\n";
    testInst->Virtual_SomeFunc(-100);
    std::cout << "Calling testInst->Virtual_SomeOtherFunc now\n";
    testInst->Virtual_SomeOtherFunc();
    std::cout << "Calling testInst->Virtual_LastVirtFunc now\n";
    testInst->Virtual_LastVirtFunc("original str arg");
    std::cout << "Calling SomeGlobalFunc now\n";
    SomeGlobalFunc(DBL_MAX);

    std::cout << "Done calling all original functions.\n"
        "testInst is located @ " << std::hex << reinterpret_cast<uintptr_t>(&testInst) << "\n"
        "\ttestInst->SomeIntField = " << testInst->SomeIntField << "\n"
        "\ttestInst->SomeFloatField = " << testInst->SomeFloatField << "\n"
        "Exiting now.\n";


    int derp = getchar();

    // clean up
    MH_Uninitialize();
    phDetour_VirtualSomeFunc->unHook();
    delete phDetour_VirtualSomeFunc;
    phHWBP_SomeGlobalFunc->unHook();
    delete phHWBP_SomeGlobalFunc;
}

void InstallAllHooks(TestClass* pTestInst)
{
    // WARNING:
    // getting the address of any function using the '&' operator
    // while incremental linking is enabled in project settings will cause a jmp thunk to be generated
    // and the returned address will be the address of the jmp thunk, not the true function address
    // so you must disable incremental linking for all build configurations
    uintptr_t addr_InstanceSomeFunc = ForceCast<uintptr_t>(&TestClass::Instance_SomeFunc);
    // NOTE:
    // relying on the order of the virtual funcs to remain the same.
    // as long as the base class definition isn't changed, this reliance isn't an issue
    void** vtable = *reinterpret_cast<void***>(pTestInst);
    uintptr_t addr_VirtualSomeFunc = reinterpret_cast<uintptr_t>(vtable[0]);
    uintptr_t addr_VirtualSomeOtherFunc = reinterpret_cast<uintptr_t>(vtable[1]);
    uintptr_t addr_LastVirtFunc = reinterpret_cast<uintptr_t>(vtable[2]);

    uintptr_t addr_SomeGlobalFunc = reinterpret_cast<uintptr_t>(&SomeGlobalFunc);

    std::cout << "Address of TestClass::Instance_SomeFunc: " << std::hex << addr_InstanceSomeFunc << "\n"
        "Address of TestClass::Virtual_SomeFunc: " << std::hex << addr_VirtualSomeFunc << "\n"
        "Address of TestClass::Virtual_SomeOtherFunc: " << std::hex << addr_VirtualSomeOtherFunc << "\n"
        "Address of TestClass::Virtual_LastVirtFunc: " << std::hex << addr_LastVirtFunc << "\n"
        "Address of SomeGlobalFunc: " << std::hex << addr_SomeGlobalFunc << "\n";


    // this is where you should play around with hooking the targets.
    // try experimenting with different hooking techniques, too.
    // feel free to write your own classes / functions to aid in your hooking techniques.
    // or add some third-party hooking libraries to the project and play around with those.

    // for this example, i've decided to use the following hooking libraries:
    // minhook (https://github.com/TsudaKageyu/minhook)
    // polyhook v2 (https://github.com/stevemk14ebr/PolyHook_2_0)

    // example: install an inline hook on TestClass::Instance_SomeFunc using minhook
    MH_STATUS status = MH_Initialize();
    if (status != MH_OK)
    {
        std::cout << "Could not initialize minhook (error: " << status << ")\n";
        return;
    }
    status = MH_CreateHookEx(reinterpret_cast<LPVOID>(addr_InstanceSomeFunc), &hk_TestClass_InstanceSomeFunc, &oInstanceSomeFunc);
    if (status != MH_OK)
    {
        std::cout << "Could not inline hook TestClass::InstanceSomeFunc with minhook (addr: " << addr_InstanceSomeFunc << " error: " << status << ")\n";
        return;
    }
    std::cout << "Installing inline hook at " << std::hex << addr_InstanceSomeFunc << "\n";
    status = MH_EnableHook(reinterpret_cast<LPVOID>(addr_InstanceSomeFunc));
    if (status != MH_OK)
    {
        std::cout << "Could not enable hook of TestClass::InstanceSomeFunc (addr: " << addr_InstanceSomeFunc << " error: " << status << ")\n";
        return;
    }
    std::cout << "Installed inline hook at " << std::hex << addr_InstanceSomeFunc << "\n";

    // example: install an inline hook on TestClass::Virtual_SomeFunc using polyhook (minhook is fine too, just demonstrating that they do the same thing)
#ifdef _M_IX86
    PLH::CapstoneDisassembler dis(PLH::Mode::x86);
    phDetour_VirtualSomeFunc = new PLH::x86Detour((char*)addr_VirtualSomeFunc, (char*)&hk_TestClass_VirtualSomeFunc, &hookPrintfTramp, dis);
#else
    PLH::CapstoneDisassembler dis(PLH::Mode::x64);
    phDetour_VirtualSomeFunc = new PLH::x64Detour((char*)addr_VirtualSomeFunc, (char*)&hk_TestClass_VirtualSomeFunc, reinterpret_cast<uint64_t*>(&oVirtualSomeFunc), dis);
#endif
    std::cout << "Installing inline hook at " << std::hex << addr_VirtualSomeFunc << "\n";
    phDetour_VirtualSomeFunc->hook();
    std::cout << "Installed inline hook at " << std::hex << addr_VirtualSomeFunc << "\n";

    // example: swap the address of pTestInst->Virtual_SomeOtherFunc with my hook address by manually writing to pTestInst's vtable
    // note: it'd probably be a good idea to freeze all other threads too,
    // but i've never experienced any crashes without doing so,
    // and this is meant to be a short demonstration anyway
    DWORD oldProtect = 0;
    if (VirtualProtect(vtable, sizeof(void*) * 4, PAGE_READWRITE, &oldProtect) == FALSE)
    {
        std::cout << "Could not make pTestInst's vtable entry writeable (addr: " << vtable[1] << " err: " << GetLastError() << ")\n";
        return;
    }
    std::cout << "Performing vtable entry swap of TestClass::VirtualSomeOtherFunc (function: " << std::hex << addr_VirtualSomeFunc << " vtable: " << vtable << ")\n";
    oVirtualSomeOtherFunc = (dTestClass_VirtualSomeOtherFunc)vtable[1];
    vtable[1] = &hk_TestClass_VirtualSomeOtherFunc;
    if (VirtualProtect(vtable, sizeof(void*) * 4, oldProtect, &oldProtect) == FALSE)
    {
        std::cout << "Could not restore pTestInst's vtable entry's page protection (addr: " << vtable[1] << " err: " << GetLastError() << ")\n";
        return;
    }
    std::cout << "Done with vtable entry swap\n";

    // example: i'll leave TestClass::Virtual_LastVirtFunc untouched (so you can compare it to the other hooked functions)
    
    // example: install a hwbp hook on SomeGlobalFunc using polyhook
    phHWBP_SomeGlobalFunc = new PLH::HWBreakPointHook((char*)addr_SomeGlobalFunc, (char*)&hk_SomeGlobalFunc, GetCurrentThread());
    std::cout << "Installing hardware breakpoint hook at " << std::hex << addr_SomeGlobalFunc << "\n";
    phHWBP_SomeGlobalFunc->hook();
    std::cout << "Installed hardware breakpoint hook at " << std::hex << addr_SomeGlobalFunc << "\n";

    std::cout << "Done installing all hooks\n";
}

#pragma region HookHandlerFunctions

int hk_TestClass_InstanceSomeFunc(TestClass* that)
{
    std::cout << "*** hk_TestClass_InstanceSomeFunc executed ***\n"
        "\targs[0]: that (implicit this) = " << std::hex << reinterpret_cast<uintptr_t>(that) << "\n"
        "\t\tthis->SomeIntField: " << that->SomeIntField << "\n"
        "\t\tthis->SomeFloatField: " << that->SomeFloatField << "\n";

    int origRetVal = oInstanceSomeFunc(that);
    return 1337;
}

int hk_TestClass_VirtualSomeFunc(TestClass* that, int derp)
{
    std::cout << "*** hk_TestClass_VirtualSomeFunc executed ***\n"
        "\targs[0]: that (implicit this) = " << std::hex << reinterpret_cast<uintptr_t>(that) << "\n"
        "\t\tthis->SomeIntField: " << that->SomeIntField << "\n"
        "\t\tthis->SomeFloatField: " << that->SomeFloatField << "\n"
        "\targs[1]: derp = " << derp << "\n";

    int origRetVal = oVirtualSomeFunc(that, derp);
    return 314159;
}

void hk_TestClass_VirtualSomeOtherFunc(TestClass* that)
{
    std::cout << "*** hk_TestClass_VirtualSomeOtherFunc executed ***\n"
        "\targs[0]: that (implicit this) = " << std::hex << reinterpret_cast<uintptr_t>(that) << "\n"
        "\t\tthis->SomeIntField: " << that->SomeIntField << "\n"
        "\t\tthis->SomeFloatField: " << that->SomeFloatField << "\n";

    oVirtualSomeOtherFunc(that);
}

bool hk_TestClass_LastVirtFunc(TestClass* that, const char* str)
{
    std::cout << "*** hk_TestClass_LastVirtFunc executed ***\n"
        "\targs[0]: that (implicit this) = " << std::hex << reinterpret_cast<uintptr_t>(that) << "\n"
        "\t\tthis->SomeIntField: " << that->SomeIntField << "\n"
        "\t\tthis->SomeFloatField: " << that->SomeFloatField << "\n"
        "\targs[1]: str = " << str << "\n";

    bool origRetVal = oLastVirtFunc(that, str);
    return false;
}

int hk_SomeGlobalFunc(double derp)
{
    std::cout << "*** hk_SomeGlobalFunc was executed ***\n"
        "\targs[0]: derp = " << derp << "\n";

    // polyhook's HWBP hook requires unhooking and calling getProtectionObject() before calling the original function
    // when the object returned by getProtectionObject() is destructed (upon exiting this function's scope) it'll call hook() again.
    phHWBP_SomeGlobalFunc->unHook();
    auto protObj = phHWBP_SomeGlobalFunc->getProtectionObject();

    int origRetVal = SomeGlobalFunc(derp);
    return 265;
}
#pragma endregion