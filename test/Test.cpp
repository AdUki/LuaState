#include "../include/LuaState.h"

#define check(code, result) \
{ \
    if (code == result) { \
        printf("OK %s == %s\n", #code, #result); \
    } \
    else { \
        printf("FAIL %s == %s\n", #code, #result); \
        assert(false); \
    } \
}

#include <iostream>

using namespace std::placeholders;

const char* sayHello()
{
    printf("Hello!\n");
    return "Hello return\n";
}

struct Foo {
    int a; int b;
    
	void setB(int value) { b = value; }
	Foo(lua::State& state) {
        
        state["Foo_setA"] = [this](int value) { a = value; };
		state["Foo_setB"] = std::function<void(int)>(std::bind(&Foo::setB, this, _1));
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // State creation doString and loadFile
    lua::State state;
    state.doString("a = 5");
    check(state["a"], 5);
    
    try {
        state.doString("a()");
        assert(false);
    } catch (lua::LoadError ex) {
        printf("%s ", ex.what());
        printf("OK\n");
    }
    
    try {
        state.doFile("bum");
        assert(false);
    } catch (lua::LoadError ex) {
        printf("%s ", ex.what());
        printf("OK\n");
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    state.doString("a = 'ahoj'");
    std::string text = state["a"];
    check(text, "ahoj");
    check(state["a"], "ahoj");
    check(state["a"], text.c_str());
    const char* cText = state["a"];
    check(strcmp(cText, "ahoj"), 0);


    state.doString("a = true");
    bool boolValue = state["a"];
    check(boolValue, true);

    state.doString("a = false");
    boolValue = state["a"];
    check(boolValue, false);

    state.doString("tab = { a = 1; b = '2'; ct = { 10, 20, 30 } }");
    check(state["tab"]["a"], 1);
    check(state["tab"]["b"], "2");
    check(state["tab"]["ct"][1], 10);
    check(state["tab"]["ct"][2], 20);
    check(state["tab"]["ct"][3], 30);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    {
        lua::Value table = state["tab"]["ct"];
        int ct1 = table[1];
        int ct2 = table[2];
        int ct3 = table[3];
        check(ct1, 10);
        check(ct2, 20);
        check(ct3, 30);
        
        {
            lua::Value table2 = state["tab"];
            int tab = table2["a"];
            check(tab, 1);
            
            try {
                ct2 = table[1];
                assert(false);
            } catch (lua::StackError exp) {
                check(ct2, 20);
            }
        }
        ct2 = table[1];
        check(ct2, 10);
    }
    
    int intValue = state["tab"]["ct"][2];
    check(intValue, 20);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    state.doString("function setGlobalTest(name, value) testGlobal = 666 end");
    state["setGlobalTest"]();
    check(state["testGlobal"], 666);

    state.doString("function setGlobal(name, value) _G[name] = value end");
    state["setGlobal"]("a", 1000000);
    check(state["a"], 1000000);
    
    state.doString("function add(a,b) return a+b end");
    intValue = state["add"](5,7);
    check(intValue, 12);

    state.doString("function logicOr(a,b) return a or b end");
    boolValue = state["logicOr"](true,false);
    check(boolValue, true);
    
    state.doString("function cat(a,b) return a .. b end");
    cText = state["cat"]("aa","bb");
//    text = state["cat"]("aa","bb");
    text = cText; // TODO: fix na string
    check(text, "aabb");

    state.doString("function multiRet() return 4.5, 10, false, 6.5 end");
    double doubleValue;
    float floatValue;
    lua::tie(doubleValue, intValue, boolValue, floatValue) = state["multiRet"]();
    check(doubleValue, 4.5);
    check(intValue, 10);
    check(boolValue, false);
    check(floatValue, 6.5);

    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    state["tab"]["a"] = 33;
    intValue = state["tab"]["a"];
    check(state["tab"]["a"], 33);
    
    state["var"] = 8;
    check(state["var"], 8);
    
    state["tab"][10] = 44;
    state["tab"][10] = 55;
    intValue = state["tab"][10];
    check(state["tab"][10], 55);
    
    state["tab"][11] = "text";
    cText = state["tab"][11];
    text = cText;
    check(text, "text");
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    state.doString("nested = { { { { 'Insane in the middle brain!' } } } }");
    
    state["nested"][1][1][1][1] = "OK";
    check(state["nested"][1][1][1][1], "OK");
    
    state["nested"][1][1][2] = "OK";
    check(state["nested"][1][1][2], "OK");
    
    state["nested"][1][1]["a"] = "OK";
    check(state["nested"][1][1]["a"], "OK");
    
    //////////////////////////////////////////////////////////////////////////////////////////////////

    state["newTable"] = 8;
    state["newTable"] = lua::Table();
    state["newTable"][1] = 5;
    check(state["newTable"][1], 5);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    bool flag = false;
    state["lambda"] = [&flag]() { flag = true; };
    state.doString("lambda()");
    check(flag, true);
    
    state["lambda"] = &sayHello;
    const char* msg = state["lambda"]();
    check(strcmp(msg, sayHello()), 0);
    
    state["lambda"] = [](int a, int b) -> int { return a + b; };
    state.doString("a = lambda(4, 8)");
    check(state["a"], 12);
    intValue = state["lambda"](2, 7);
    check(intValue, 9);
    
    state["lambda"] = []() -> std::tuple<LuaType::Integer, LuaType::String> {
        return std::tuple<LuaType::Integer, LuaType::String>(23, "abc");
    };
    
    lua::tie(intValue,  cText) = state["lambda"]();
    check(intValue, 23);
    check(strcmp(cText, "abc") , 0);
    
    Foo foo(state);
    state["Foo_setA"](10);
    state["Foo_setB"](20);
    check(foo.a, 10);
    check(foo.b, 20);
    
    typedef std::tuple<std::function<int()>, std::function<int()>, std::function<int()>> FunctionTuples1;
    state["getFncs"] = []() -> FunctionTuples1 {
        return FunctionTuples1([] () -> int { return 100; },
                              [] () -> int { return 200; },
                              [] () -> int { return 300; });
    };
    int a, b, c;
    typedef std::tuple<std::function<void(int)>, std::function<void(int)>, std::function<void(int)>> FunctionTuples2;
    state["setValues"] = [&a, &b, &c]() -> FunctionTuples2 {
        return FunctionTuples2([&] (int value) { a = value; },
                               [&] (int value) { b = value; },
                               [&] (int value) { c = value; });
    };
    
    state.doString("fnc1, fnc2, fnc3 = getFncs()"
                   "setA, setB, setC = setValues()"
                   "setA(fnc1()); setB(fnc2()); setC(fnc3())");
    check(a, 100);
    check(b, 200);
    check(c, 300);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    check(state.flushStack(), 0);
    return 0;
}
