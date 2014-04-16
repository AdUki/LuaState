//
//  LuaValue.h
//  LuaState
//
//  Created by Simon Mikuda on 17/03/14.
//
//  See LICENSE and README.md files

#pragma once

#include <tuple>
#include <cstring>

#include "./LuaStack.h"
#include "./LuaFunctor.h"
#include "./LuaFunction.h"

namespace lua {
    
    class Value;
    typedef const Value Ref;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    class Value
    {
        std::shared_ptr<lua_State> _luaState;
        int _stackTop;
        mutable int _pushedValues;

        Value(const std::shared_ptr<lua_State>& luaState)
        : _luaState(luaState)
        , _stackTop(stack::top(_luaState.get()))
        , _pushedValues(0)
        {
        }
        
    public:
        // constructors
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        Value(const std::shared_ptr<lua_State>& luaState, const char* name)
        : _luaState(luaState)
        , _stackTop(stack::top(_luaState.get()))
        , _pushedValues(1)
        {
            stack::get_global(_luaState.get(), name);
        }
        
        ~Value() {
            if (_luaState != nullptr)
                stack::settop(_luaState.get(), _stackTop);
        }
        
        Value(const Value& value) = delete;
        Value(Value&&) = default;
        
        // operator overloads
        //////////////////////////////////////////////////////////////////////////////////////////////////
        Value& operator= (Value& value) = delete;
        Value& operator= (Value &&) = default;

        template<typename T>
        Value operator[](T key) const {
            Value value(_luaState);
            stack::get(_luaState.get(), _stackTop + _pushedValues, key);
            value._pushedValues = 1;
            
            return std::move(value);
        }
        
        template<typename T>
        Value&& operator[](T key) {
            ++_pushedValues;
            stack::get(_luaState.get(), _stackTop + _pushedValues - 1, key);
            
            return std::move(*this);
        }
        
        template<typename ... Ts>
        Function operator()(Ts... args) const {
            --_pushedValues;
            
            stack::push(_luaState.get(), args...);
            return Function(_luaState, sizeof...(args), false);
        }
        
        template<typename ... Ts>
        Function call(Ts... args) const {
            --_pushedValues;
            
            stack::push(_luaState.get(), args...);
            return Function(_luaState, sizeof...(args), false);
        }
        
        template<typename ... Ts>
        Function protectedCall(Ts... args) const {
            --_pushedValues;
            
            stack::push(_luaState.get(), args...);
            return Function(_luaState, sizeof...(args), true);
        }
        
        template<typename T>
        operator T() const {
            --_pushedValues;
            
            auto retValue(stack::read<T>(_luaState.get(), -1));
            stack::pop(_luaState.get(), 1);
            return retValue;
        }

        template<typename K, typename T>
        void set(const K& key, const T& value) const {
            stack::push(_luaState.get(), key);
            stack::push(_luaState.get(), value);
            lua_settable(_luaState.get(), -3);
        }
        
        // other functions
        //////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename T>
        bool is() const {
            return stack::check<T>(_luaState.get(), -1);
        }
        
        template <typename T>
        bool get(T& value) const {
            if (is<T>() == false)
                return false;
            else {
                value = stack::read<T>(_luaState.get(), -1);
                return true;
            }
        }
        
        template <typename T>
        T* getPtr() const {
            return static_cast<T*>(Pointer(*this));
        }
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

