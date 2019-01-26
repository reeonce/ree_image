#pragma once

#include <string>


template<typename T> std::string reeToString(T t) {
    return std::to_string(t);
}

namespace ree {

class UnitTestFunc {
public:
    UnitTestFunc(const std::string &klass, const std::string &name);
    std::string Klass() const;
    std::string Name() const;
    bool Failed() const;
    std::string Description() const;
    
    virtual void Run() = 0;
protected:
    void Fail(const std::string &line, const std::string &checkDescription);
private:
    std::string klass_;
    std::string name_;
    bool failed_ = false;
    std::string failDescription_;
};

class UnitTestImpl;
class UnitTest {
public:
    static UnitTest &Instance();

    ~UnitTest();

    void RegisterFunc(UnitTestFunc *func);
    void Init(int argc, const char **argv);
    void RunAllTests();

private:
    UnitTest();
    UnitTestImpl *impl_;
};
}

#define R_TEST_F(klass, name) \
    class name##UnitTestFunc : public ree::UnitTestFunc {\
name##UnitTestFunc() : UnitTestFunc(#klass, #name) {\
            UnitTest::Instance().RegisterFunc(this);\
        }\
        void Run() override;\
        static name##UnitTestFunc instance_;\
    };\
    name##UnitTestFunc name##UnitTestFunc::instance_;\
    void name##UnitTestFunc::Run()

#define R_RUN_TESTS() \
    ree::UnitTest::Instance().RunAllTests()

#define R_TEST_INIT(argc, argv) \
    ree::UnitTest::Instance().Init(argc, argv)

#define R_STRINGIZE2(x) #x
#define R_STRINGIZE(x) R_STRINGIZE2(x)

#define R_ASSERT_TRUE(val) \
    if (val != true) { \
        Fail(__FILE__ ": " R_STRINGIZE(__LINE__),\
        #val "\nNot TRUE");\
    }

#define R_ASSERT_FALSE(val) \
    if (val == true) { \
        Fail(__FILE__ ": " R_STRINGIZE(__LINE__),\
        #val "\nNot FALSE");\
    }

#define R_ASSERT_EQ(val1, val2) \
    {\
    auto v1 = val1;\
    auto v2 = val2;\
    if (v1 != v2) { \
        Fail(__FILE__ ": " R_STRINGIZE(__LINE__),\
        std::string(#val1" == "#val2 "\nActual: ") +\
        reeToString(v1) + "\nExpect: " + reeToString(v2));\
    }\
    }
