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
        /// Pointer of Lua state
        lua_State* _luaState;
        detail::DeallocQueue* _deallocQueue;
        
        /// Key of referenced value in LUA_REGISTRYINDEX
        int _refKey;
        
        void createRefKey() {
            _refKey = luaL_ref(_luaState, LUA_REGISTRYINDEX);
        }
        
    public:
        
        Ref() : _luaState(nullptr) {}
        
        // Copy and move constructors just use operator functions
        Ref(const Value& value) { operator=(value); }
        Ref(Value&& value) { operator=(value); }
        
        ~Ref() {
            luaL_unref(_luaState, LUA_REGISTRYINDEX, _refKey);
        }
        
        /// Copy assignment. Creates lua::Ref from lua::Value.
        void operator= (const Value& value) {
            _luaState = value._stack->state;
            _deallocQueue = value._stack->deallocQueue;

            // Duplicate top value
            lua_pushvalue(_luaState, -1);
            
            // Create reference to registry
            createRefKey();
	    }

        /// Move assignment. Creates lua::Ref from lua::Value from top of stack and pops it
        void operator= (Value&& value) {
            _luaState = value._stack->state;
            _deallocQueue = value._stack->deallocQueue;
            
            if (value._stack->pushed > 0)
                value._stack->pushed -= 1;
            else
                value._stack->top -= 1;
            
            // Create reference to registry
            createRefKey();
	    }
        
        /// Creates lua::Value from lua::Ref
        ///
        /// @return lua::Value with referenced value on stack
        Value unref() const {
            lua_rawgeti(_luaState, LUA_REGISTRYINDEX, _refKey);
            return Value(std::make_shared<detail::StackItem>(_luaState, _deallocQueue, stack::top(_luaState) - 1, 1, 0));
        }
        
        bool isInitialized() const { return _luaState != nullptr; }
    };
    
    
}