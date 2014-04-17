//
//  LuaState.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files

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
