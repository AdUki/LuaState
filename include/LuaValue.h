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

namespace lua {
    
    class Value;
    class State;
    class Ref;
    template <typename ... Ts> class Return;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// This is class for:
    /// * querying values from lua tables,
    /// * setting values to lua tables,
    /// * calling values as functions,
    /// * checking value type.
    class Value
    {
        friend class State;
        friend class Ref;
        template <typename ... Ts> friend class Return;
        
        std::shared_ptr<lua_State> _luaState;
        
        /// Indicates number of pushed values to stack on lua::Value creating
        int _stackTop;
        
        /// Indicates number pushed values which were pushed by this lua::Value instance
        mutable int _pushedValues;

        /// Constructor for crating lua::Ref instances
        ///
        /// @param luaState     Shared pointer of Lua state
        Value(const std::shared_ptr<lua_State>& luaState)
        : _luaState(luaState)
        , _stackTop(stack::top(_luaState.get()))
        , _pushedValues(0)
        {
        }
        
        /// Constructor for lua::State class. Whill get global in _G table with name
        ///
        /// @param luaState     Shared pointer of Lua state
        /// @param name         Key of global value
        Value(const std::shared_ptr<lua_State>& luaState, const char* name)
        : _luaState(luaState)
        , _stackTop(stack::top(_luaState.get()))
        , _pushedValues(1)
        {
            stack::get_global(_luaState.get(), name);
        }
        
    public:
        
        /// Enable to initialize empty Value, so we can set it up later
        Value() {}
        
        /// Upon deletion we will restore stack to original value
        ~Value() {
            if (_luaState != nullptr)
                stack::settop(_luaState.get(), _stackTop);
        }
        
        // Default move constructor and operator
        Value(Value&&) = default;
        Value& operator= (Value &&) = default;
        
        // Deleted copy constructor and operator
        Value(const Value& value) = delete;
        Value& operator= (Value& value) = delete;

        /// With this function we will create lua::Ref instance
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename T>
        Value operator[](T key) const & {
            Value value(_luaState);
            stack::get(_luaState.get(), _stackTop + _pushedValues, key);
            value._pushedValues = 1;
            
            return std::move(value);
        }
        
        /// While chaining [] operators we will call this function multiple times and can query nested tables.
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename T>
        Value&& operator[](T key) && {
            ++_pushedValues;
            stack::get(_luaState.get(), _stackTop + _pushedValues - 1, key);
            
            return std::move(*this);
        }
        
        /// Call given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename ... Ts>
        Value operator()(Ts... args) & {
            
            Value value(_luaState);
            stack::push(_luaState.get(), args...);
            
            lua_call(_luaState.get(), sizeof...(Ts), LUA_MULTRET);
            value._pushedValues += stack::top(_luaState.get()) - (_stackTop + _pushedValues);
            
            return std::move(value);
        }
        
        /// Call given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename ... Ts>
        Value&& operator()(Ts... args) && {
            
            stack::push(_luaState.get(), args...);

            lua_call(_luaState.get(), sizeof...(Ts), LUA_MULTRET);
            _pushedValues += stack::top(_luaState.get()) - (_stackTop + _pushedValues);
            
            return std::move(*this);
        }
        
        /// Protected call of given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename ... Ts>
        Value call(Ts... args) & {
            
            Value value(_luaState);
            stack::push(_luaState.get(), args...);
            
            if (lua_pcall(_luaState.get(), sizeof...(Ts), LUA_MULTRET, 0)) {
                const char* error = lua_tostring(_luaState.get(), -1);
                lua_pop(_luaState.get(), 1);
                throw RuntimeError(error);
            }
            value._pushedValues += stack::top(_luaState.get()) - (_stackTop + _pushedValues);
            
            return std::move(value);
        }
        
        /// Protected call of given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename ... Ts>
        Value&& call(Ts... args) && {
            
            stack::push(_luaState.get(), args...);
            
            if (lua_pcall(_luaState.get(), sizeof...(Ts), LUA_MULTRET, 0)) {
                const char* error = lua_tostring(_luaState.get(), -1);
                lua_pop(_luaState.get(), 1);
                throw RuntimeError(error);
            }
            _pushedValues += stack::top(_luaState.get()) - (_stackTop + _pushedValues);
            
            return std::move(*this);
        }
        
        /// Cast operator. Enables to pop values from stack and store it to variables
        ///
        /// @return Any value of type from LuaPrimitives.h
        template<typename T>
        operator T() const {
            --_pushedValues;
            
            auto retValue(stack::read<T>(_luaState.get(), -1));
            stack::pop(_luaState.get(), 1);
            return retValue;
        }

        /// Set values to table to the given key.
        ///
        /// @param key      Key to which value will be stored
        /// @param value    Value to be stored to table on given key
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename K, typename T>
        void set(const K& key, const T& value) const {
            stack::push(_luaState.get(), key);
            stack::push(_luaState.get(), value);
            lua_settable(_luaState.get(), -3);
        }

        /// Check if queryied value is some type from LuaPrimitives.h file
        ///
        /// @return true if yes false if no
        template <typename T>
        bool is() const {
            return stack::check<T>(_luaState.get(), -1);
        }
        
        /// First check if lua::Value is type T and if yes stores it to value
        ///
        /// @param value    Reference to variable where will be stored result if types are right
        ///
        /// @return true if value was given type and stored to value false if not
        template <typename T>
        bool get(T& value) const {
            if (is<T>() == false)
                return false;
            else {
                value = stack::read<T>(_luaState.get(), -1);
                return true;
            }
        }
        
        /// Will get pointer casted to given template type
        ///
        /// @return Pointer staticaly casted to given template type
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

