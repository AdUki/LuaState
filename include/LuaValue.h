//
//  LuaValue.h
//  LuaState
//
//  Created by Simon Mikuda on 17/03/14.
//
//

#pragma once

#include <tuple>
#include <cstring>
#include "./LuaStack.h"
#include "./LuaPrimitives.h"
#include "./LuaFunction.h"

namespace lua {

    //////////////////////////////////////////////////////////////////////////////////////////////////
    class Value
    {
        std::shared_ptr<lua_State> _luaState;
        
        mutable int _pushedValues;
        const int _stackTop;
        
        //////////////////////////////////////////////////////////////////////////////////////////////////
        void checkStack() const {
            if (stack::numberOfPushedValues(_luaState.get()) - _stackTop != _pushedValues) {
                throw StackError(stack::numberOfPushedValues(_luaState.get()), _pushedValues);
            }
        }
        
    public:
        // constructors
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        Value(const std::shared_ptr<lua_State>& luaState, const char* name)
        : _luaState(luaState)
        , _pushedValues(2)
        , _stackTop(stack::numberOfPushedValues(_luaState.get())) {
            stack::push(luaState.get(), name);
            stack::get(_luaState.get(), LUA_GLOBALSINDEX, name);
        }
        
        ~Value() {
            if (_luaState != nullptr)
                stack::pop(_luaState.get(), _pushedValues);
        }
        
        Value(const Value&) = delete;
        Value(Value&&) = default;
        
        // operator overloads
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        Value& operator= (const Value &) = delete;
        Value& operator= (Value &&) = default;
        
        template<typename T>
        Value&& operator[](T key) {
            checkStack();
            
            stack::push(_luaState.get(), key);
            stack::get(_luaState.get(), -2, key);
            _pushedValues += 2;
            
            return std::move(*this);
        }

        Function operator()() const {
            checkStack();
            _pushedValues -= 1;
            return Function(_luaState, 0);
        }
        
        template<typename... Ts>
        Function operator()(Ts... args) const {
            checkStack();
            _pushedValues -= 1;
            stack::push(_luaState.get(), args...);
            return Function(_luaState, sizeof...(args));
        }
        
        template<typename T>
        operator T() const {
            checkStack();
            auto retValue = stack::read<T>(_luaState.get(), -1);
            stack::pop(_luaState.get(), 2);
            _pushedValues -= 2;
            return retValue;
        }
        
        template<typename T>
        void operator= (const T& value) const {
            checkStack();
            
            stack::pop(_luaState.get(), 1);
            
            if (_pushedValues == 2) {
                LuaType::String name = stack::pop_front<LuaType::String>(_luaState.get());
                stack::push(_luaState.get(), value);
                lua_setglobal(_luaState.get(), name);
            }
            else {
                stack::push(_luaState.get(), value);
                lua_settable(_luaState.get(), -3);
            }
            _pushedValues -= 2;
        }

        // other functions
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
    };
    
    // compare operators
    //////////////////////////////////////////////////////////////////////////////////////////////////
    inline bool operator==(const Value &value, const char *string)        { return strcmp(value, string) == 0; }
    inline bool operator==(const char *string, const Value &value)        { return strcmp(value, string) == 0; }
    inline bool operator==(const Value &value, const std::string& string) { return strcmp(value, string.c_str()) == 0; }
    inline bool operator==(const std::string& string, const Value &value) { return strcmp(value, string.c_str()) == 0; }
    
    template <typename T>
    inline bool operator==(const Value &stateValue, const T& value) {
        return T(stateValue) == value;
    }
    template <typename T>
    inline bool operator==(const T& value, const Value &stateValue) {
        return T(stateValue) == value;
    }
    
}

