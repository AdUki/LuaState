//
//  LuaRef.h
//  LuaState
//
//  Created by Simon Mikuda on 17/04/14.
//
//  See LICENSE and README.md files

#pragma once

namespace lua {
    
    /// Reference to Lua value. Can be created from any lua::Value
    class Ref
    {
        /// Shared pointer of Lua state
        std::shared_ptr<lua_State> _luaState;
        
        /// Key of referenced value in LUA_REGISTRYINDEX
        int _refKey;
        
    public:
        
        /// Destructor releases registered value
        ~Ref() {
            // Unregister reference from registry
            luaL_unref(_luaState.get(), LUA_REGISTRYINDEX, _refKey);
        }

        // Copy and move constructors just use operator functions
        Ref(const Value& value) { operator=(value); }
        Ref(Value&& value) { operator=(value); }
        
        /// Copy assignment. Creates lua::Ref from lua::Value.
        void operator= (const Value& value) {
            _luaState = value._luaState;

            // Duplicate top value
            lua_pushvalue(_luaState.get(), -1);
            
            // Create reference to registry
            _refKey = luaL_ref(_luaState.get(), LUA_REGISTRYINDEX);
	    }

        /// Move assignment. Creates lua::Ref from lua::Value.
        void operator= (Value&& value) {
            _luaState = value._luaState;
            
            if (value._pushedValues > 0)
                value._pushedValues -= 1;
            else
                value._stackTop -= 1;
            
            // Create reference to registry
            _refKey = luaL_ref(_luaState.get(), LUA_REGISTRYINDEX);
	    }
        
        /// Creates lua::Value from lua::Ref
        ///
        /// @return lua::Value with referenced value on stack
        Value get() const {
            
            Value value(_luaState);
            
            lua_rawgeti(_luaState.get(), LUA_REGISTRYINDEX, _refKey);
            value._pushedValues = 1;
            
            return std::move(value);
        }
    };
    
    
}