//////////////////////////////////////////////////////////////////////////////////////////////////
/*!

 \mainpage Lightweight Lua binding library for C++11
 
 ### Setting it up
 
 Just create \ref{lua::State} variable, which will initialize lus_State and loads up standard libraries. It will close state automatically in destructor.
 
 ~~~~~~~~~~~~~{.cpp}
 #include <luastate.h>
 int main() {
     lua::State state;
     state.doString("print 'Hello world!'");
 }
 ~~~~~~~~~~~~~
 
 ### Reading values
 
 Reading values from Lua state is very simple. It is using templates, so type information is required. You will be using \ref{lua::Value} class.
 
 ~~~~~~~~~~~~~{.cpp}
 state.doString("number = 100; text = 'hello'");
 int number =state["number"];
 std::string text = state["text"];
 ~~~~~~~~~~~~~
 
 When reading values from tables, you just chain [] operators.
 
 ~~~~~~~~~~~~~{.cpp}
 state.doString("table = { a = 1, b = { 2 }, c = 3}");
 int a = state["table"]["a"];
 int b = state["table"]["b"][1];
 int c = state["table"]["c"];
 ~~~~~~~~~~~~~
 
 ### Calling functions
 
 You can call lua functions with () operator with various number of arguments while returning none, one or more values. You will be using \ref{lua::Function} class.
 
 ~~~~~~~~~~~~~{.cpp}
 state.doString("function setFoo() foo = "hello" end");
 state.doString("function getFoo() return foo end");
 
 state["setFoo"]()
 std::string text = state["getFoo"]()
 
 state.doString("function add(x, y) return x + y end");
 int result = state["add"](1,2);
 
 state.doString("function iWantMore() return 20, 13.8, 'MORE' end");
 float number;
 std::string text;
 lua::tie(result, number, text) = state["iWantMore"]();
 ~~~~~~~~~~~~~
 
 ### Setting values
 
 Is also pretty straightforward...
 
 ~~~~~~~~~~~~~{.cpp}
 state.doString("table = { a = 1, b = { 2 }, c = 3}");
 state["table"].set("a", 100);
 state["table"]["b"].set(1, 200);
 state["table"].set("c", 300);
 
 state.set("newTable", lua::Table());
 state["newTable"].set(1, "a");
 state["newTable"].set(2, "b");
 state["newTable"].set(3, "c");
 ~~~~~~~~~~~~~
 
 ### Setting functions
 
 You can bind C functions, lambdas and std::functions with bind. These instances are managed by Lua garbage collector and will be destroyed when you will lost last reference in Lua state to them.
 
 ~~~~~~~~~~~~~{.cpp}
 void sayHello() { printf("Hello!\n"); }
 state.set("cfunction", &sayHello);
 state["cfunction"](); // Hello!
 
 int value = 20;
 state.set("lambda", [value](int a, int b) -> int { return (a*b)/value; } )
 int result = state["lambda"](12, 5); // result = 3
 ~~~~~~~~~~~~~
 
 They can return one or more values with use of std::tuple. For example, when you want to register more functions, you can return bundled in tuple...
 
 ~~~~~~~~~~~~~{.cpp}
 state.set("getFncs", []()
 -> std::tuple<std::function<int()>, std::function<int()>, std::function<int()>> {
     return {
         []() -> int { return 100; },
         []() -> int { return 200; },
         []() -> int { return 300; }
     };
 } );
 state.doString("fnc1, fnc2, fnc3 = getFncs()"
 "print(fnc1(), fnc2(), fnc3())"); // 100 200 300
 ~~~~~~~~~~~~~
 
 You can easily register your classes functions with `this` pointer passing to lambda capture or bind...
 
 ~~~~~~~~~~~~~{.cpp}
 struct Foo {
 int a; int b;
 
 void setB(int value) { b = value; }
 Foo(lua::State& state) {
     state.set("Foo_setA", [this](int value) { a = value; } );
     state.set("Foo_setB", std::function<void(int)>(std::bind(&Foo::setB, this, _1)) );
 }
 };
 ~~~~~~~~~~~~~
 
 ### Managing C++ classes by garbage collector
 
 It is highly recommended to use shared pointers and then you will have garbage collected classes in C++. Objects will exist util there is last instance of shared pointer and they will be immediately released when all shared pointer instances are gone.
 
 Our resource:
 
 ~~~~~~~~~~~~~{.cpp}
 struct Resource {
 Resource() { printf("New resource\n"); }
 ~Resource() { printf("Released resource\n"); }
 
 void doStuff() { printf("Working..."); }
 };
 ~~~~~~~~~~~~~
 
 Resource using and released by garbage collector:
 
 ~~~~~~~~~~~~~{.cpp}
 std::shared_ptr<Resource> resource = std::make_shared<Resource>(); // New resource
 state.set("useResource", [resource]() { resource->doStuff(); } );
 resource.reset();
 
 state.doString("useResource()"); // Working
 state.doString("useResource = nil; collectgarbage()"); // Released resource
 ~~~~~~~~~~~~~

*/
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LuaState.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files
//////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cassert>
#include <string>
#include <functional>
#include <memory>
#include <tuple>

#include <lua.hpp>

#ifdef LUASTATE_DEBUG_MODE
#   define LUASTATE_DEBUG_LOG(format, ...) printf(format, ## __VA_ARGS__)
#else
#   define LUASTATE_DEBUG_LOG(format, ...)
#endif

#include "./LuaException.h"
#include "./LuaValue.h"
#include "./LuaReturn.h"
#include "./LuaRef.h"

namespace lua {
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// Class that hold lua interpreter state. Lua state is managed by shared pointer which also is copied to lua::Ref values.
    class State
    {
        /// Shared pointer takes care of automaticaly closing Lua state when all instances pointing to it are gone
        std::shared_ptr<lua_State> _luaState;
        
    public:
        
        /// Constructor creates new state and stores it to shared pointer.
        ///
        /// @param loadLibs     If we want to open standard libraries - function luaL_openlibs
        State(bool loadLibs) {
            
            lua_State* luaState = luaL_newstate();
            assert(luaState != NULL);
            
            if (loadLibs)
                luaL_openlibs(luaState);
            
            // We will create metatable for Lua functors for memory management and actual function call
            luaL_newmetatable(luaState, "luaL_Functor");
            
            // Set up metatable call operator for functors
            lua_pushcfunction(luaState, [](lua_State* luaState) -> int {
                BaseFunctor* functor = *(BaseFunctor **)luaL_checkudata(luaState, 1, "luaL_Functor");;
                return functor->call(luaState);
            });
            lua_setfield(luaState, -2, "__call");
            
            // Set up metatable garbage collection for functors
            lua_pushcfunction(luaState, [](lua_State* luaState) -> int {
                BaseFunctor* functor = *(BaseFunctor **)luaL_checkudata(luaState, 1, "luaL_Functor");;
                delete functor;
                return 0;
            });
            lua_setfield(luaState, -2, "__gc");
            
            // Pop metatable
            lua_pop(luaState, 1);
            
            // Set up destructor for lua state, function lua_close
            _luaState = std::shared_ptr<lua_State>(luaState, std::bind(&lua_close, luaState));
        }
        
        /// Constructor creates new state stores it to shared pointer and loads standard libraries
        State() : State(true) {}
        
        /// Query global values from Lua state
        ///
        /// @return Some value with type lua::Type
        Value operator[](lua::String name) {
            return Value(_luaState, name);
        }
        
        /// Deleted compare operator
        bool operator==(Value &other) = delete;
        
        /// Sets global value to Lua state
        ///
        /// @param key      Stores value to _G[key]
        /// @param value    Value witch will be stored to _G[key]
        template<typename T>
        void set(lua::String key, const T& value) const {
            stack::push(_luaState.get(), value);
            lua_setglobal(_luaState.get(), key);
        }
        
        /// Executes file text on Lua state
        ///
        /// @throws lua::LoadError      When file cannot be found or loaded
        /// @throws lua::RuntimeError   When there is runtime error
        ///
        /// @param filePath File path indicating which file will be executed
        void doFile(const std::string& filePath) {
            if (luaL_loadfile(_luaState.get(), filePath.c_str())) {
                std::string message = stack::read<std::string>(_luaState.get(), -1);
                stack::pop(_luaState.get(), 1);
                throw LoadError(message);
            }
            if (lua_pcall(_luaState.get(), 0, LUA_MULTRET, 0)) {
                std::string message = stack::read<std::string>(_luaState.get(), -1);
                stack::pop(_luaState.get(), 1);
                throw RuntimeError(message);
            }
        }
        
        /// Execute string on Lua state
        ///
        /// @throws lua::LoadError      When string cannot be loaded
        /// @throws lua::RuntimeError   When there is runtime error
        ///
        /// @param string   Command which will be executed
        void doString(const std::string& string) {
            if (luaL_loadstring(_luaState.get(), string.c_str())) {
                std::string message = stack::read<std::string>(_luaState.get(), -1);
                stack::pop(_luaState.get(), 1);
                throw LoadError(message);
            }
            if (lua_pcall(_luaState.get(), 0, LUA_MULTRET, 0)) {
                std::string message = stack::read<std::string>(_luaState.get(), -1);
                stack::pop(_luaState.get(), 1);
                throw RuntimeError(message);
            }
        }

#ifdef LUASTATE_DEBUG_MODE
        /// Flush all elements from stack
        ///
        /// @return Number of flushed items
        int flushStack() {
            int count = stack::top(_luaState.get());
            LUASTATE_DEBUG_LOG("Flushed %d elements from stack\n", count);
            lua_settop(_luaState.get(), 0);
            return count;
        }
#endif
        
        /// Get shared pointer of Lua state
        ///
        /// @return Shared pointer of Lua state
        std::shared_ptr<lua_State> getState() { return _luaState; }
    };
}
