//
//  Traits.h
//  LuaState
//
//  Created by Simon Mikuda on 22/03/14.
//
//  See LICENSE and README.md files

#define LUASTATE_DEBUG_MODE

#include "../include/LuaState.h"

#define check(code, result) \
{ \
    if (code == result) { \
        printf("line %d: OK %s == %s\n", __LINE__, #code, #result); \
    } \
    else { \
        printf("line %d: FAIL %s == %s\n", __LINE__, #code, #result); \
        assert(false); \
    } \
}

#include <iostream>

using namespace std::placeholders;

//////////////////////////////////////////////////////////////////////////////////////////////////
const char* getHello()
{
    return "Hello return\n";
}

//////////////////////////////////////////////////////////////////////////////////////////////////
struct Resource {
    static int refCounter;
    
    Resource() {
        ++refCounter;
    }
    
    ~Resource() {
        --refCounter;
    }
};
int Resource::refCounter = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////
struct Foo {
    int a; int b;
    
	void setB(int value) { b = value; }
    
	Foo(lua::State& state) {
        
        state.set("Foo_setA", [this](int value) { a = value; } );
		state.set("Foo_setB", std::function<void(int)>(std::bind(&Foo::setB, this, _1)) );
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
    } catch (lua::RuntimeError ex) {
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
    check(state.flushStack(), 0);

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
    check(state.flushStack(), 0);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    {
        lua::CValue table = state["tab"]["ct"];
        lua::stack::dump(state.getState().get());
        int ct1 = table[1];
        int ct2 = table[2];
        int ct3 = table[3];
        check(ct1, 10);
        check(ct2, 20);
        check(ct3, 30);
        
        lua::CValue table2 = state["tab"];
        int tab = table2["a"];
        check(tab, 1);
        
        ct3 = table[1];
        ct2 = table[3];
        ct1 = table[2];
        check(ct3, 10);
        check(ct2, 30);
        check(ct1, 20);
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
    text = lua::String(state["cat"]("aa","bb")); // TODO: fix na string
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
    
    state["tab"].set("a", 33);
    intValue = state["tab"]["a"];
    check(state["tab"]["a"], 33);
    
    state.set("var", 8);
    check(state["var"], 8);
    
    state["tab"].set(10, 44);
    state["tab"].set(10, 55);
    intValue = state["tab"][10];
    check(state["tab"][10], 55);
    
    state["tab"].set(11, "text");
    cText = state["tab"][11];
    text = cText;
    check(text, "text");
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    state.doString("nested = { { { { 'Insane in the middle brain!' } } } }");
    
    state["nested"][1][1][1].set(1, "OK");
    check(state["nested"][1][1][1][1], "OK");
    
    state["nested"][1][1].set(2, "OK");
    check(state["nested"][1][1][2], "OK");
    
    state["nested"][1][1].set("a", "OK");
    check(state["nested"][1][1]["a"], "OK");
    
    //////////////////////////////////////////////////////////////////////////////////////////////////

    state.set("newTable", 8);
    state.set("newTable", lua::Table());
    state["newTable"].set(1, 5);
    check(state["newTable"][1], 5);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////

    bool flag = false;
    state.set("lambda", [&flag]() { flag = true; } );
    state.doString("lambda()");
    check(flag, true);
    
    state.set("lambda", &getHello);
    const char* msg = state["lambda"]();
    check(strcmp(msg, getHello()), 0);
    
    state.set("lambda", [](int a, int b) -> int { return a + b; } );
    state.doString("a = lambda(4, 8)");
    check(state["a"], 12);
    intValue = state["lambda"](2, 7);
    check(intValue, 9);
    
    state.set("lambda", []() -> std::tuple<lua::Integer, lua::String> {
        return std::tuple<lua::Integer, lua::String>(23, "abc");
    });
    
    lua::tie(intValue,  cText) = state["lambda"]();
    check(intValue, 23);
    check(strcmp(cText, "abc") , 0);
    
    Foo foo(state);
    state["Foo_setA"](10);
    state["Foo_setB"](20);
    check(foo.a, 10);
    check(foo.b, 20);
    
    state.set("getFncs", []()
    -> std::tuple<std::function<int()>, std::function<int()>, std::function<int()>> {
        return {
            [] () -> int { return 100; },
            [] () -> int { return 200; },
            [] () -> int { return 300; }
        };
    });
    int a, b, c;
    state.set("setValues", [&a, &b, &c]()
    -> std::tuple<std::function<void(int)>, std::function<void(int)>, std::function<void(int)>> {
        return {
            [&] (int value) { a = value; },
            [&] (int value) { b = value; },
            [&] (int value) { c = value; }
        };
    });
    
    state.doString("fnc1, fnc2, fnc3 = getFncs()"
                   "setA, setB, setC = setValues()"
                   "setA(fnc1()); setB(fnc2()); setC(fnc3())");
    check(a, 100);
    check(b, 200);
    check(c, 300);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    state.set("goodFnc", [](){ });
    state["goodFnc"]();
    state["goodFnc"].call();
    state["goodFnc"].protectedCall().execute();
    
    flag = false;
    state.doString("badFnc = function(a,b) local var = a .. b end");
    try {
        state["badFnc"].protectedCall(3).execute();
        assert(false);
    } catch (lua::RuntimeError ex) {
        flag = true;
    }
    check(flag, true);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    Foo* fooPtr = new Foo(state);
    fooPtr->a = 13;
    state.set<lua::Pointer>("fooPtr", fooPtr);
    fooPtr = state["fooPtr"].getPtr<Foo>();
    check(fooPtr->a, 13);
    
    fooPtr->b = 22;
    state.set<lua::Pointer>("fooPtr", fooPtr);
    fooPtr = static_cast<Foo*>(lua::Pointer(state["fooPtr"]));
    check(fooPtr->b, 22);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    state.doString("collectgarbage()");
    
    std::shared_ptr<Resource> res = std::make_shared<Resource>();
    state.set("fun", [res]() {} );
    res.reset();
    
    state.doString("fun2 = fun; fun = nil; collectgarbage()");
    check(Resource::refCounter, 1);

    state.doString("fun2 = nil; collectgarbage()");
    check(Resource::refCounter, 0);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    {
        state.doString("passToFunction = { a = 5, nested = { b = 4 } }");
        lua::CValue luaValue = state["passToFunction"];
        check(luaValue["a"], 5);
        check(luaValue["nested"]["b"], 4);
        check(luaValue["a"], 5);
        
        auto fnc = [] (const lua::CValue& value) {
            check(value["a"], 5);
            check(value["nested"]["b"], 4);
            check(value["a"], 5);
            
            lua::CValue nestedLuaValue = value["nested"];
            check(nestedLuaValue["b"], 4);
        };
        fnc(luaValue);
        fnc(state["passToFunction"]);
        
        check(luaValue["a"], 5);
        check(luaValue["nested"]["b"], 4);
        check(luaValue["a"], 5);
        
        lua::CValue nestedLuaValue = luaValue["nested"];
        check(nestedLuaValue["b"], 4);
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    {
        lua::CValue ref = state["sdjkflaksdjfla"];
        check(ref.is<lua::Null>(), true);
    }
    check(state["passToFunction"]["baaalalalala"].is<lua::Null>(), true);
    check(state["passToFunction"]["a"].is<lua::Null>(), false);
    check(state["passToFunction"]["nested"]["baaalalalala"].is<lua::Null>(), true);
    check(state["passToFunction"]["nested"]["b"].is<lua::Null>(), false);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    void* pointer = &intValue;
    state.set("pointer", pointer);
    state.set("text", "my text");
    
    check(state["pointer"].is<lua::Pointer>(), true);
    check(state["a"].is<lua::Pointer>(), false);
    check(state["tab"].is<lua::Pointer>(), false);
    check(state["a"].is<lua::Integer>(), true);
    check(state["a"].is<lua::String>(), false);
    check(state["text"].is<lua::String>(), true);
    check(state["a"].is<lua::Table>(), false);
    check(state["tab"].is<lua::Table>(), true);
    check(state["a"].is<lua::Null>(), false);
    check(state["tab"].is<lua::Null>(), false);
    check(state["this_is_nil_value"].is<lua::Null>(), true);
    
    if (state["a"].get(intValue)) {
        check(intValue, 12);
    }
    else assert(false);
    
    if (state["text"].get(cText)) {
        check(strcmp(cText, "my text"), 0);
    }
    else assert(false);
    
    check(state["this_is_nil_value"].get(intValue), false);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    {
        state.doString("tab = { a = 1; b = 'my text'; ct = { 10, 20, 30 } }");
        
        check(state["tab"]["a"].is<lua::Integer>() && state["tab"]["b"].is<lua::String>(), true);
        
        lua::CValue tabRef = state["tab"];
        
        check(tabRef["a"].is<lua::Integer>(), true);
        check(tabRef["b"].is<lua::String>(), true);
        check(tabRef["a"].is<lua::Integer>() && tabRef["b"].is<lua::String>(), true);
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    check(state["no function here"].is<lua::Callable>(), false);
    check(state["a"].is<lua::Callable>(), false);
    check(state["tab"].is<lua::Callable>(), false);
    check(state["print"].is<lua::Callable>(), true);
    check(state["io"]["write"].is<lua::Callable>(), true);
    check(state["setGlobalTest"].is<lua::Callable>(), true);
    check(state["goodFnc"].is<lua::Callable>(), true);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    lua::Ref ref = state["tab"]["a"];
    lua::Ref tabRef = state["tab"];
    
    check(ref.unref(), 1);
    check(tabRef.unref()["a"], 1);
    
    lua::Ref copyRef = ref;
    check(copyRef.unref(), 1);
    
    copyRef = tabRef;
    check(copyRef.unref()["a"], 1);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    check(state.flushStack(), 0);
    
    return 0;
}
