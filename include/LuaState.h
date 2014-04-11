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

namespace lua {
    
    class State
    {
        std::shared_ptr<lua_State> _luaState;
        
    public:
        // constructors
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        State() {
            lua_State* luaState = luaL_newstate();
            assert(luaState != NULL);
            
            luaL_openlibs(luaState);
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
            
            lua_pop(luaState, 1);
            
            // Set up destructor for lua state
            _luaState = std::shared_ptr<lua_State>(luaState, std::bind(&lua_close, luaState));
        }
        
        ~State() {
        }
        
        // operators overloads
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        Value operator[](lua::String name) {
            return Value(_luaState, name);
        }
        
        // Allow automatic casting when used in comparisons
        bool operator==(Value &other) = delete;
        
        // other functions
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        void doFile(const std::string& filePath) {
            if (luaL_dofile(_luaState.get(), filePath.c_str())) {
                std::string message = stack::read<std::string>(_luaState.get(), -1);
                stack::pop(_luaState.get(), 1);
                throw LoadError(message);
            }
        }
        void doString(const std::string& string) {
            if (luaL_dostring(_luaState.get(), string.c_str())) {
                std::string message = stack::read<std::string>(_luaState.get(), -1);
                stack::pop(_luaState.get(), 1);
                throw LoadError(message);
            }
        }

#ifdef LUASTATE_DEBUG_MODE
        /// @returns Number of flushed items
        int flushStack() {
            int count = stack::numberOfPushedValues(_luaState.get());
            LUASTATE_DEBUG_LOG("Flushed %d elements from stack\n", count);
            lua_settop(_luaState.get(), 0);
            return count;
        }
#endif
        
        std::shared_ptr<lua_State> getState() { return _luaState; }
    };
}
