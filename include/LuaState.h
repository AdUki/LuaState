////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LuaState.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// Compatibility with non-clang compilers.
#ifndef __has_feature
#   define __has_feature(x) 0
#endif

#ifdef LUASTATE_DEBUG_MODE
#   define LUASTATE_DEBUG_LOG(format, ...) printf(format "\n", ## __VA_ARGS__)
#   define LUASTATE_ASSERT(condition)      assert(condition)
#else
#   define LUASTATE_DEBUG_LOG(format, ...)
#   define LUASTATE_ASSERT(condition)
#endif

#include <cassert>
#include <string>
#include <functional>
#include <memory>
#include <tuple>
#include <cstring>
#include <cmath>

#include <lua.hpp>

#include "./Traits.h"

#include "./LuaPrimitives.h"
#include "./LuaStack.h"
#include "./LuaException.h"
#include "./LuaStackItem.h"
#include "./LuaValue.h"
#include "./LuaReturn.h"
#include "./LuaFunctor.h"
#include "./LuaRef.h"

namespace lua {
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    /// Class that hold lua interpreter state. Lua state is managed by pointer which also is copied to lua::Ref values.
    class State
    {
        /// Class takes care of automaticaly closing Lua state when in destructor
        lua_State* _luaState;
        
        /// Class deletes DeallocQueue in destructor
        detail::DeallocQueue* _deallocQueue;
        
        /// Function for metatable "__call" field. It calls stored functor pushes return values to stack.
        ///
        /// @pre In Lua C API during function calls lua_State moves stack index to place, where first element is our userdata, and next elements are returned values
        static int metatableCallFunction(lua_State* luaState) {
            BaseFunctor* functor = *(BaseFunctor **)luaL_checkudata(luaState, 1, "luaL_Functor");;
            return functor->call(luaState);
        }
        
        /// Function for metatable "__gc" field. It deletes captured variables from stored functors.
        static int metatableDeleteFunction(lua_State* luaState) {
            BaseFunctor* functor = *(BaseFunctor **)luaL_checkudata(luaState, 1, "luaL_Functor");;
            delete functor;
            return 0;
        }
        
        lua::Value executeLoadedFunction(int index) const {
//            bool executed;
//            try {
//                executed = lua_pcall(_luaState, 0, LUA_MULTRET, 0) == 0;
//            } catch (lua::TypeMismatchError ex) {
//                throw ex;
//            }
//            
//            if (!executed)
            if (lua_pcall(_luaState, 0, LUA_MULTRET, 0))
                throw RuntimeError(_luaState);
            
            int pushedValues = stack::top(_luaState) - index;
            return lua::Value(std::make_shared<detail::StackItem>(_luaState, _deallocQueue, index, pushedValues, pushedValues > 0 ? pushedValues - 1 : 0));
        }
        
        void initialize(bool loadLibs) {
            _deallocQueue = new detail::DeallocQueue();
            _luaState = luaL_newstate();
            assert(_luaState != nullptr);
            
            if (loadLibs)
                luaL_openlibs(_luaState);
            
            
            // We will create metatable for Lua functors for memory management and actual function call
            luaL_newmetatable(_luaState, "luaL_Functor");
            
            // Set up metatable call operator for functors
            lua_pushcfunction(_luaState, &State::metatableCallFunction);
            lua_setfield(_luaState, -2, "__call");
            
            // Set up metatable garbage collection for functors
            lua_pushcfunction(_luaState, &State::metatableDeleteFunction);
            lua_setfield(_luaState, -2, "__gc");
            
            // Pop metatable
            lua_pop(_luaState, 1);
        }
        
    public:
        
        /// Constructor creates new state and stores it to pointer.
        ///
        /// @param loadLibs     If we want to open standard libraries - function luaL_openlibs
        State(bool loadLibs) { initialize(loadLibs); }
        
        /// Constructor creates new state stores it to pointer and loads standard libraries
        State() { initialize(true); }
        
        ~State() {
            lua_close(_luaState);
            delete _deallocQueue;
        }
        
        // State is non-copyable
        State(const State& other) = delete;
        State& operator=(const State&) = delete;
        
        /// Query global values from Lua state
        ///
        /// @return Some value with type lua::Type
        Value operator[](lua::String name) const {
            return Value(_luaState, _deallocQueue, name);
        }
        
        /// Deleted compare operator
        bool operator==(Value &other) = delete;
        
        /// Sets global value to Lua state
        ///
        /// @param key      Stores value to _G[key]
        /// @param value    Value witch will be stored to _G[key]
        template<typename T>
        void set(lua::String key, T value) const {
            stack::push(_luaState, std::forward<T>(value));
            lua_setglobal(_luaState, key);
        }
        
        /// Executes file text on Lua state
        ///
        /// @throws lua::LoadError      When file cannot be found or loaded
        /// @throws lua::RuntimeError   When there is runtime error
        ///
        /// @param filePath File path indicating which file will be executed
        lua::Value doFile(const std::string& filePath) const {
            int stackTop = stack::top(_luaState);
            
            if (luaL_loadfile(_luaState, filePath.c_str()))
                throw LoadError(_luaState);
            
            return executeLoadedFunction(stackTop);
        }
        
        /// Execute string on Lua state
        ///
        /// @throws lua::LoadError      When string cannot be loaded
        /// @throws lua::RuntimeError   When there is runtime error
        ///
        /// @param string   Command which will be executed
        lua::Value doString(const std::string& string) const {
            int stackTop = stack::top(_luaState);
            
            if (luaL_loadstring(_luaState, string.c_str()))
                throw LoadError(_luaState);

            return executeLoadedFunction(stackTop);
        }

#ifdef LUASTATE_DEBUG_MODE
        
        /// Flush all elements from stack and check ref counting
        void checkMemLeaks() {
            
            bool noLeaks = true;
            
            // Check if there are any values from stack, should be zero
            int count = stack::top(_luaState);
            if (count != 0) {
                LUASTATE_DEBUG_LOG("There are %d elements in stack:", count);
                stack::dump(_luaState);
                noLeaks = false;
            }
            
            // Dealloc queue should be empty
            if (!_deallocQueue->empty()) {
                LUASTATE_DEBUG_LOG("Deallocation queue has %lu elements:", _deallocQueue->size());
                while (!_deallocQueue->empty()) {
                    LUASTATE_DEBUG_LOG("[stackCap = %d, numElements = %d]", _deallocQueue->top().stackCap, _deallocQueue->top().numElements);
                    _deallocQueue->pop();
                }
                noLeaks = false;
            }
            assert(noLeaks);
        }
        
        void stackDump() const {
            lua::stack::dump(_luaState);
        }
#endif
        
        /// Get pointer of Lua state
        ///
        /// @return Pointer of Lua state
        lua_State* getState() { return _luaState; }
        
        
        //////////////////////////////////////////////////////////////////////////////////////////////
        // Conventional setting functions
        
        void setCStr(lua::String key, const char* value) const {
            set<const char*>(key, value);
        }
        
        void setData(lua::String key, const char* value, size_t length) const {
            stack::push_str(_luaState, value, length);
            lua_setglobal(_luaState, key);
        }
        
        void setString(lua::String key, const std::string& string) const {
            setData(key, string.c_str(), string.length());
        }
        
        void set(lua::String key, const std::string& value) const {
            setString(key, value);
        }
        
        void setNumber(lua::String key, lua::Number number) const {
            set<lua::Number>(key, number);
        }
        
        void setInt(lua::String key, int number) const {
            set<int>(key, number);
        }
        
        void setUnsigned(lua::String key, unsigned number) const {
            set<unsigned>(key, number);
        }
        
        void setFloat(lua::String key, float number) const {
            set<float>(key, number);
        }
        
        void setDouble(lua::String key, double number) const {
            set<double>(key, number);
        }
    };
}
