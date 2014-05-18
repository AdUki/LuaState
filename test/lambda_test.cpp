//
//  state_tests.h
//  LuaState
//
//  Created by Simon Mikuda on 16/04/14.
//
//  See LICENSE and README.md files

#include "test.h"

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
    lua::State state;

    // TODO: organize code
    
    int intValue;
    lua::String cText;
    
    bool flag = false;
    state.set("lambda", [&flag]() { flag = true; } );
    state.doString("lambda()");
    assert(flag == true);
    
    state.set("lambda", &getHello);
    const char* msg = state["lambda"]();
    assert(strcmp(msg, getHello()) == 0);
    
    state.set("lambda", [](int a, int b) -> int { return a + b; } );
    state.doString("a = lambda(4, 8)");
    assert(state["a"] == 12);
    intValue = state["lambda"](2, 7);
    assert(intValue == 9);
    
    state.set("lambda", []() -> std::tuple<lua::Integer, lua::String> {
        return std::tuple<lua::Integer, lua::String>(23, "abc");
    });
    
    lua::tie(intValue, cText) = state["lambda"]();
    assert(intValue == 23);
    assert(strcmp(cText, "abc") == 0);
    
    Foo foo(state);
    state["Foo_setA"](10);
    state["Foo_setB"](20);
    assert(foo.a == 10);
    assert(foo.b == 20);
    
    state.set("getFncs", []()
              -> std::tuple<std::function<int()>, std::function<int()>, std::function<int()>> {
                  return {
                      [] () -> int { return 100; },
                      [] () -> int { return 200; },
                      [] () -> int { return 300; },
                  };
              });
    int a, b, c;
    state.set("setValues", [&a, &b, &c]()
              -> std::tuple<std::function<void(int)>, std::function<void(int)>, std::function<void(int)>> {
                  return {
                      [&] (int value) { a = value; },
                      [&] (int value) { b = value; },
                      [&] (int value) { c = value; },
                  };
              });
    
    state.doString("fnc1, fnc2, fnc3 = getFncs()"
                   "setA, setB, setC = setValues()"
                   "setA(fnc1()); setB(fnc2()); setC(fnc3())");
    assert(a == 100);
    assert(b == 200);
    assert(c == 300);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    state.set("goodFnc", [](){ });
    state["goodFnc"]();
    state["goodFnc"].call();
    
    flag = false;
    state.doString("badFnc = function(a,b) local var = a .. b end");
    try {
        lua::Value aaa = state["goodFnc"];
        state["badFnc"].call(3);
        assert(false);
    } catch (lua::RuntimeError ex) {
        flag = true;
    }
    assert(flag == true);
    
    {
        state.doString("passToFunction = { a = 5, nested = { b = 4 } }");
        lua::Value luaValue = state["passToFunction"];
        assert(luaValue["a"] == 5);
        
        assert(luaValue["nested"]["b"] == 4);
        assert(luaValue["a"] == 5);
        
        auto fnc = [] (const lua::Value& value) {
            assert(value["a"] == 5);
            assert(value["nested"]["b"] == 4);
            assert(value["a"] == 5);
            
            lua::Value nestedLuaValue = value["nested"];
            assert(nestedLuaValue["b"] == 4);
        };
        fnc(luaValue);
        fnc(state["passToFunction"]);
        
        assert(luaValue["a"] == 5);
        assert(luaValue["nested"]["b"] == 4);
        assert(luaValue["a"] == 5);
        
        lua::Value nestedLuaValue = luaValue["nested"];
        assert(nestedLuaValue["b"] == 4);
    }
    
    // assert if stack leaked, we must pop 0 values
    assert(state.flushStack() == 0);

    return 0;
}
