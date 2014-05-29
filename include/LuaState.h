////////////////////////////////////////////////////////////////////////////////////////////////
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
#include <cstring>
#include <cmath>

#include <lua.hpp>

#ifdef LUASTATE_DEBUG_MODE
#   define LUASTATE_DEBUG_LOG(format, ...) printf(format "\n", ## __VA_ARGS__)
#   define LUASTATE_ASSERT(condition)      assert(condition)
#else
#   define LUASTATE_DEBUG_LOG(format, ...)
#   define LUASTATE_ASSERT(condition)
#endif

#include "./DeallocStackQueue.h"
#include "./Traits.h"

#include "./LuaPrimitives.h"
#include "./LuaStack.h"
#include "./LuaException.h"
#include "./LuaValue.h"
#include "./LuaReturn.h"
#include "./LuaFunctor.h"
#include "./LuaRef.h"

namespace lua {
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
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
        
    public:
        
        /// Constructor creates new state and stores it to pointer.
        ///
        /// @param loadLibs     If we want to open standard libraries - function luaL_openlibs
        State(bool loadLibs) {
            
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
        
        /// Constructor creates new state stores it to pointer and loads standard libraries
        State() : State(true) {}
        
        ~State() {
            lua_close(_luaState);
            delete _deallocQueue;
        }
        
        /// Query global values from Lua state
        ///
        /// @return Some value with type lua::Type
        Value operator[](lua::String name) {
            return Value(_luaState, _deallocQueue, name);
        }
        
        /// Deleted compare operator
        bool operator==(Value &other) = delete;
        
        /// Sets global value to Lua state
        ///
        /// @param key      Stores value to _G[key]
        /// @param value    Value witch will be stored to _G[key]
        template<typename T>
        void set(lua::String key, const T& value) const {
            stack::push(_luaState, value);
            lua_setglobal(_luaState, key);
        }
        
        /// Executes file text on Lua state
        ///
        /// @throws lua::LoadError      When file cannot be found or loaded
        /// @throws lua::RuntimeError   When there is runtime error
        ///
        /// @param filePath File path indicating which file will be executed
        void doFile(const std::string& filePath) {
            if (luaL_loadfile(_luaState, filePath.c_str())) {
                std::string message = stack::read<std::string>(_luaState, -1);
                stack::pop(_luaState, 1);
                throw LoadError(message);
            }
            if (lua_pcall(_luaState, 0, LUA_MULTRET, 0)) {
                std::string message = stack::read<std::string>(_luaState, -1);
                stack::pop(_luaState, 1);
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
            if (luaL_loadstring(_luaState, string.c_str())) {
                std::string message = stack::read<std::string>(_luaState, -1);
                stack::pop(_luaState, 1);
                throw LoadError(message);
            }
            if (lua_pcall(_luaState, 0, LUA_MULTRET, 0)) {
                std::string message = stack::read<std::string>(_luaState, -1);
                stack::pop(_luaState, 1);
                throw RuntimeError(message);
            }
        }

#ifdef LUASTATE_DEBUG_MODE
        
        /// Flush all elements from stack and check ref counting
        void checkMemLeaks() {
            
            LUASTATE_DEBUG_LOG("Reference counter is %d", REF_COUNTER);
            
            int count = stack::top(_luaState);
            LUASTATE_DEBUG_LOG("Flushed %d elements from stack:", count);
            stack::dump(_luaState);
            lua_settop(_luaState, 0);

            LUASTATE_DEBUG_LOG("Deallocation queue has %lu elements", _deallocQueue->size());
            assert(_deallocQueue->empty());
            
            // Check for memory leaks during ref counting, should be zero
            assert(REF_COUNTER == 0);
            
            // Check if there are any values from stack, should be zero
            assert(count == 0);
        }
        
        void stackDump() {
            lua::stack::dump(_luaState);
        }
#endif
        
        /// Get pointer of Lua state
        ///
        /// @return Pointer of Lua state
        lua_State* getState() { return _luaState; }
    };
}
