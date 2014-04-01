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

#ifdef LUASTATE_DEBUG_MODE
#   define CHECK_STACK() \
    if (stack::numberOfPushedValues(_luaState.get()) - _stackTop != _pushedValues) { \
        throw StackError(_luaState.get(), stack::numberOfPushedValues(_luaState.get()) - _stackTop, _pushedValues); \
    }
#else
#   define CHECK_STACK()
#endif

namespace lua {

    //////////////////////////////////////////////////////////////////////////////////////////////////
    class Value
    {
        std::shared_ptr<lua_State> _luaState;
        
        mutable int _pushedValues;
        
#ifdef LUASTATE_DEBUG_MODE
        int _stackTop;
#endif
        
    public:
        // constructors
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        Value(const std::shared_ptr<lua_State>& luaState, const char* name)
        : _luaState(luaState)
        , _pushedValues(2)
#ifdef LUASTATE_DEBUG_MODE
        , _stackTop(stack::numberOfPushedValues(_luaState.get()))
#endif
        {
            stack::push(luaState.get(), name);
            stack::get_global(_luaState.get(), name);
        }
        
        ~Value() {
            if (_luaState != nullptr)
                stack::pop(_luaState.get(), _pushedValues);
        }
        
        Value(const Value&) = default;
        Value(Value&&) = default;
        
        // operator overloads
        //////////////////////////////////////////////////////////////////////////////////////////////////
        Value& operator= (Value& value) = delete;
        Value& operator= (Value &&) = default;

        template<typename T>
        Value operator[](T key) const & {
            CHECK_STACK();
            _pushedValues += 2;
            
            stack::push(_luaState.get(), key);
            stack::get(_luaState.get(), -2, key);
            return Value(*this);
        }
        
        template<typename T>
        Value&& operator[](T key) {
            CHECK_STACK();
            _pushedValues += 2;
            
            stack::push(_luaState.get(), key);
            stack::get(_luaState.get(), -2, key);
            return std::move(*this);
        }
        
        template<typename ... Ts>
        Function operator()(Ts... args) const {
            CHECK_STACK();
            _pushedValues -= 1;
            
            stack::push(_luaState.get(), args...);
            return Function(_luaState, sizeof...(args), false);
        }
        
        template<typename ... Ts>
        Function call(Ts... args) const {
            CHECK_STACK();
            _pushedValues -= 1;
            
            stack::push(_luaState.get(), args...);
            return Function(_luaState, sizeof...(args), false);
        }
        
        template<typename ... Ts>
        Function protectedCall(Ts... args) const {
            CHECK_STACK();
            _pushedValues -= 1;
            
            stack::push(_luaState.get(), args...);
            return Function(_luaState, sizeof...(args), true);
        }
        
        template<typename T>
        operator T() const {
            CHECK_STACK();
            _pushedValues -= 2;
            
            auto retValue(stack::read<T>(_luaState.get(), -1));
            stack::pop(_luaState.get(), 2);
            return retValue;
        }
        
        template<typename T>
        void operator= (const T& value) const {
            CHECK_STACK();
            
            stack::pop(_luaState.get(), 1);
            
            if (_pushedValues == 2) {
                lua::String name = stack::pop_front<lua::String>(_luaState.get());
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

        template <typename T>
        void setPointer(T* pointer) {
            operator=(static_cast<void*>(pointer));
        }
        
        template <typename T>
        T* getPointer() {
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

