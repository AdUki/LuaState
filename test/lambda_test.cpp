//
//  state_tests.h
//  LuaState
//
//  Created by Simon Mikuda on 16/04/14.
//
//  See LICENSE and README.md files

#include "test.h"

using namespace std::placeholders;

//////////////////////////////////////////////////////////////////////////////////////////////
const char* getHello()
{
    return "Hello return\n";
}

int subValues(int a, int b)
{
    return a - b;
}

//////////////////////////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////////////////////////////
struct Foo {
    int a; int b;
    
	void setB(int value) { b = value; }
    
	Foo(lua::State& state) {
        
        state.set("Foo_setA", [this](int value) { a = value; } );
        state.set("Foo_setB", std::function<void(int)>(std::bind(&Foo::setB, this, _1)) );
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // Create Lua state
    lua::State state;
    
    int intValue;
    lua::String cText;
    
    // Test normal function bind
    state.set("lambda", &getHello);
    assert(strcmp(state["lambda"](), getHello()) == 0);
    
    // Test lambda captures
    bool flag = false;
    state.set("lambda", [&flag]() { flag = true; } );
    state.doString("lambda()");
    assert(flag == true);
    
    // Test function and lambda arguments
    state.set("lambda", &subValues);
    assert(state["lambda"](8, 5) == 3);
    
    state.set("lambda", [](int a, int b, int c, int d) -> int {
        return a + b + c + d;
    });
    state.doString("a = lambda(4, 8, 12, 14)");
    assert(state["a"] == 38);
    
    // Test direct passing to functions
    {
        lua::Value value = state["lambda"];
        assert(value(state["a"], 1, 0, 1) == 40);
        assert(value(1, state["a"], 0, 1) == 40);
        assert(value(1, 0, state["a"], 1) == 40);
        assert(value(1, 0, 1, state["a"]) == 40);
        assert(value(state["a"], state["a"], state["a"], state["a"]) == 152);
    }
    
    assert(state["lambda"](state["a"], 1, 0, 1) == 40);
    assert(state["lambda"](1, state["a"], 0, 1) == 40);
    assert(state["lambda"](1, 0, state["a"], 1) == 40);
    assert(state["lambda"](1, 0, 1, state["a"]) == 40);
    assert(state["lambda"](state["a"], state["a"], state["a"], state["a"]) == 152);
    
    // Test when we provide less arguments than we need
    state.set("lambda", [&intValue](lua::Value a, lua::Value b, lua::Value c, lua::Value d) {
        intValue =
        (a.is<lua::Number>() ? a : 0) +
        (b.is<lua::Number>() ? b : 0) +
        (c.is<lua::Number>() ? c : 0) +
        (d.is<lua::Number>() ? d : 0);
    });
    state.doString("lambda()");
    assert(intValue == 0);
    state.doString("lambda(5)");
    assert(intValue == 5);
    state.doString("lambda(4, 8)");
    assert(intValue == 12);
    state.doString("lambda(1, 2, 3)");
    assert(intValue == 6);
    state.doString("lambda(1, 2, 3, 4)");
    assert(intValue == 10);
    {
        lua::Value nilValue1 = state["novaluehere"];
        lua::Value nilValue2 = state["novaluehere"];
        lua::Value nilValue3 = state["novaluehere"];
        state.doString("lambda(1,2)");
        assert(intValue == 3);
    }
    
    // Test passing reference to capture
    state.set("lambda", [&intValue](int a, int b, int c, int d) {
        intValue = a + b + c + d;
    });
    state.doString("lambda(4, 8, 12, 14)");
    assert(intValue == 38);
    
    // Test when we provide more arguments than we need
    state.doString("lambda(1,2,3,4,5,6,7,8,9)");
    assert(intValue == 10);
    {
        lua::Value nilValue1 = state["novaluehere"];
        lua::Value nilValue2 = state["novaluehere"];
        lua::Value nilValue3 = state["novaluehere"];
        state.doString("lambda(1,2,3,4,5,6,7,8,9)");
        assert(intValue == 10);
    }
    
    // Type mismatch exception
//    try {
//        state["lambda"]("hello");
//        assert(false);
//    } catch (lua::TypeMismatchError ex) {
//        LUASTATE_DEBUG_LOG("%s", ex.what());
//        LUASTATE_DEBUG_LOG("Exception cought... OK");
//    }
//    
//    try {
//        state.doString("lambda('hello')");
//        assert(false);
//    } catch (lua::TypeMismatchError ex) {
//        LUASTATE_DEBUG_LOG("%s", ex.what());
//        LUASTATE_DEBUG_LOG("Exception cought... OK");
//    } catch (lua::RuntimeError ex) {
//        LUASTATE_DEBUG_LOG("%s", ex.what());
//        LUASTATE_DEBUG_LOG("Exception cought... OK");
//    }
    
    // Test multi return
    state.set("lambda", []() -> std::tuple<lua::Integer, lua::String> {
        return std::tuple<lua::Integer, lua::String>(23, "abc");
    });
    
    lua::tie(intValue, cText) = state["lambda"]();
    assert(intValue == 23);
    assert(strcmp(cText, "abc") == 0);
    
    // Test class binding
    Foo foo(state);
    state["Foo_setA"](10);
    state["Foo_setB"](20);
    assert(foo.a == 10);
    assert(foo.b == 20);
    
    state.set("getFncs", []()
              -> std::tuple<std::function<int()>, std::function<int()>, std::function<int()>> {
                // Error in gcc 4.8.1, we must again specify tuple parameters
                  return std::make_tuple<std::function<int()>, std::function<int()>, std::function<int()>>(
                      [] () -> int { return 100; },
                      [] () -> int { return 200; },
                      [] () -> int { return 300; }
                  );
              });
    int a, b, c;
    state.set("setValues", [&a, &b, &c]()
              -> std::tuple<std::function<void(int)>, std::function<void(int)>, std::function<void(int)>> {
                  return std::make_tuple(
                      [&] (int value) { a = value; },
                      [&] (int value) { b = value; },
                      [&] (int value) { c = value; }
                  );
              });
    
    state.doString("fnc1, fnc2, fnc3 = getFncs()"
                   "setA, setB, setC = setValues()"
                   "setA(fnc1()); setB(fnc2()); setC(fnc3())");
    assert(a == 100);
    assert(b == 200);
    assert(c == 300);
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    
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
    
//    std::string hello = state["mystr"];
//    hello = state["mystr"];
//    
    {   // LuaState treats empty return values as nil
        lua::Value value;
        
        value = state.doString("return nil");
        assert(value.is<lua::Nil>());
        
        value = state.doString("return");
        assert(value.is<lua::Nil>());
    }
    
    state.checkMemLeaks();
    return 0;
}
