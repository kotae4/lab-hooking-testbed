#pragma once

class TestVMethodClass
{
public:
    // do not change the order of these first 3. doing so will break retrieving their addresses in main.cpp:InstallAllHooks.
    virtual int Virtual_SomeFunc(int derp) { return 0; };
    virtual void Virtual_SomeOtherFunc() {};
    virtual bool Virtual_LastVirtFunc(const char* str) { return false; };
};

class TestClass : public TestVMethodClass
{
public:
    int SomeIntField;
    float SomeFloatField;

    int Instance_SomeFunc();
    int Virtual_SomeFunc(int derp) override;
    void Virtual_SomeOtherFunc() override;
    bool Virtual_LastVirtFunc(const char* str) override;
};