//
//  LuaValue.h
//  LuaState
//
//  Created by Simon Mikuda on 17/03/14.
//
//  See LICENSE and README.md files

#pragma once

namespace lua {
    
    class Value;
    class State;
    class Ref;
    template <typename ... Ts> class Return;

    //////////////////////////////////////////////////////////////////////////////////////////////
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
        
        std::shared_ptr<detail::StackItem> _stack;
        
        /// Constructor for lua::State class. Whill get global in _G table with name
        ///
        /// @param luaState     Pointer of Lua state
        /// @param deallocQueue Queue for deletion values initialized from given luaState
        /// @param name         Key of global value
        Value(lua_State* luaState, detail::DeallocQueue* deallocQueue, const char* name)
        : _stack(std::make_shared<detail::StackItem>(luaState, deallocQueue, stack::top(luaState), 1, 0))
        {
            stack::get_global(_stack->state, name);
        }
        
        template<typename ... Ts>
        void callFunction(bool protectedCall, Ts... args) const {
            
            // Function must be on top of stack
            LUASTATE_ASSERT(stack::check<Callable>(_stack->state, stack::top(_stack->state)));
            
            stack::push(_stack->state, args...);
            
            if (protectedCall) {
                if (lua_pcall(_stack->state, sizeof...(Ts), LUA_MULTRET, 0))
                    throw RuntimeError(_stack->state);
            }
            else
                lua_call(_stack->state, sizeof...(Ts), LUA_MULTRET);
        }
        
        template<typename ... Ts>
        Value executeFunction(bool protectedCall, Ts... args) const {
            
            int stackTop = stack::top(_stack->state);
            
            // We will duplicate Lua function value, because it will get poped from stack
            lua_pushvalue(_stack->state, _stack->top + _stack->pushed - _stack->grouped);
            
            callFunction(protectedCall, args...);
            int returnedValues = stack::top(_stack->state) - stackTop;
            
            LUASTATE_ASSERT(returnedValues >= 0);
            
            return Value(std::make_shared<detail::StackItem>(_stack->state, _stack->deallocQueue, stackTop, returnedValues, returnedValues == 0 ? 0 : returnedValues - 1));
        }
        
        template<typename ... Ts>
        Value&& executeFunction(bool protectedCall, Ts... args) {
            
            int stackTop = stack::top(_stack->state);
            
            // we check if there are not pushed values before function
            if (_stack->top + _stack->pushed < stackTop) {
                
                _stack->deallocQueue->push(detail::DeallocStackItem(_stack->top, _stack->pushed));

                lua_pushvalue(_stack->state, _stack->top + 1);
                
                _stack->top = stackTop;
                _stack->pushed = 1;
                _stack->grouped = 0;
                
                ++stackTop;
            }
            
            // StackItem top must same as top of current stack
            LUASTATE_ASSERT(_stack->top + _stack->pushed == stack::top(_stack->state));
            
            callFunction(protectedCall, args...);

            _stack->grouped = stack::top(_stack->state) - stackTop;
            _stack->pushed += _stack->grouped;

            LUASTATE_ASSERT(_stack->pushed >= 0);
            
            return std::move(*this);
        }
        
    public:
        
        /// Enable to initialize empty Value, so we can set it up later
        Value() : _stack(nullptr) {
        }

        /// Constructor for returning values from functions and for creating lua::Ref instances
        ///
        /// @param stackItem Prepared stack item
        Value(std::shared_ptr<detail::StackItem>&& stackItem)
        : _stack(stackItem)
        {
        }
        
        /// With this function we will create lua::Ref instance
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename T>
        Value operator[](T key) const {
            stack::get(_stack->state, _stack->top + _stack->pushed - _stack->grouped, key);
            return Value(std::make_shared<detail::StackItem>(_stack->state, _stack->deallocQueue, stack::top(_stack->state) - 1, 1, 0));
        }
        
#if __has_feature(cxx_reference_qualified_functions)
        
        /// While chaining [] operators we will call this function multiple times and can query nested tables.
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename T>
        Value&& operator[](T key) && {
            stack::get(_stack->state, _stack->top + _stack->pushed - _stack->grouped, key);
            ++_stack->pushed;
            
            _stack->grouped = 0;
            
            return std::forward<Value>(*this);
        }
        
#endif
        
        /// Call given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename ... Ts>
        Value operator()(Ts... args) const {
            return  executeFunction(false, args...);
        }
        
        /// Protected call of given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename ... Ts>
        Value call(Ts... args) const {
            return executeFunction(true, args...);
        }
        
#if __has_feature(cxx_reference_qualified_functions)
        
        /// Call given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename ... Ts>
        Value&& operator()(Ts... args) && {
            return executeFunction(false, args...);
        }
        
        /// Protected call of given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename ... Ts>
        Value&& call(Ts... args) && {
            return executeFunction(true, args...);
        }
        
#endif
        
        template<typename T>
        T to() const {
            return std::forward<T>(stack::read<T>(_stack->state, _stack->top + _stack->pushed - _stack->grouped));
        }
        
        /// Cast operator. Enables to pop values from stack and store it to variables
        ///
        /// @return Any value of type from LuaPrimitives.h
        template<typename T>
        operator T() const {
            return to<T>();
        }
        
        /// Set values to table to the given key.
        ///
        /// @param key      Key to which value will be stored
        /// @param value    Value to be stored to table on given key
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename K, typename T>
        void set(K key, T value) const {
            stack::push(_stack->state, key);
            stack::push(_stack->state, std::forward<T>(value));
            lua_settable(_stack->state, _stack->top + _stack->pushed - _stack->grouped);
        }

        /// Check if queryied value is some type from LuaPrimitives.h file
        ///
        /// @return true if yes false if no
        template <typename T>
        bool is() const {
            return stack::check<T>(_stack->state, _stack->top + _stack->pushed - _stack->grouped);
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
                value = stack::read<T>(_stack->state, _stack->top + _stack->pushed - _stack->grouped);
                return true;
            }
        }
            
        /// @returns Value position on stack
        int getStackIndex() const {
            assert(_stack->pushed > 0);
            return _stack->top + 1;
        }
        
        //////////////////////////////////////////////////////////////////////////////////////////////
        // Conventional conversion functions

        // to
        
        const char* toCStr() const {
            return to<const char*>();
        }
        
        std::string toString() const {
            return to<lua::String>();
        }
        
        lua::Number toNumber() const {
            return to<lua::Number>();
        }
        
        int toInt() const {
            return to<int>();
        }
        
        unsigned toUnsigned() const {
            return to<unsigned>();
        }
        
        float toFloat() const {
            return to<float>();
        }
        
        double toDouble() const {
            return to<double>();
        }

        /// Will get pointer casted to given template type
        ///
        /// @return Pointer staticaly casted to given template type
        template <typename T>
        T* toPtr() const {
            return static_cast<T*>(Pointer(*this));
        }
        
        // get
        
        bool getCStr(const char*& cstr) const {
            return get<const char*>(cstr);
        }
        
        bool getString(std::string& string) const {
            lua::String cstr;
            bool success = get<lua::String>(cstr);
            
            if (success)
                string = cstr;
            
            return success;
        }
        
        bool getNumber(lua::Number number) const {
            return get<lua::Number>(number);
        }
        
        bool getInt(int number) const {
            return get<int>(number);
        }
        
        bool getUnsigned(unsigned number) const {
            return get<unsigned>(number);
        }
        
        bool getFloat(float number) const {
            return get<float>(number);
        }
        
        bool getDouble(double number) const {
            return get<double>(number);
        }
        
        template <typename T>
        T* getPtr(T*& pointer) const {
            lua::Pointer cptr;
            bool success = get<lua::Pointer>(cptr);
            
            if (success)
                pointer = static_cast<T*>(Pointer(*this));
            
            return success;
        }
        
        //////////////////////////////////////////////////////////////////////////////////////////////
        // Conventional setting functions
        
        template<typename K>
        void setCStr(K key, const char* value) const {
            set<const char*>(key, value);
        }
        
        template<typename K>
        void setData(K key, const char* value, size_t length) const {
            stack::push(_stack->state, key);
            stack::push_str(_stack->state, value, length);
            lua_settable(_stack->state, _stack->top + _stack->pushed - _stack->grouped);
        }
        
        template<typename K>
        void setString(K key, const std::string& string) const {
            stack::push(_stack->state, key);
            stack::push_str(_stack->state, string.c_str(), string.length());
            lua_settable(_stack->state, _stack->top + _stack->pushed - _stack->grouped);
        }
        
        int length() const {
#if LUA_VERSION_NUM > 501
            return lua_rawlen(_stack->state, _stack->top + _stack->pushed - _stack->grouped);
#else
            return lua_objlen(_stack->state, _stack->top + _stack->pushed - _stack->grouped);
#endif
        }

        template<typename K>
        void set(K key, const std::string& value) const {
            setString<lua::String>(key, value);
        }
        
        template<typename K>
        void setNumber(K key, lua::Number number) const {
            set<lua::Number>(key, number);
        }
        
        template<typename K>
        void setInt(K key, int number) const {
            set<int>(key, number);
        }
        
        template<typename K>
        void setUnsigned(K key, unsigned number) const {
            set<unsigned>(key, number);
        }
        
        template<typename K>
        void setFloat(K key, float number) const {
            set<float>(key, number);
        }
        
        template<typename K>
        void setDouble(K key, double number) const {
            set<double>(key, number);
        }
        
    };
    
    // compare operators
    //////////////////////////////////////////////////////////////////////////////////////////////
    
    template <typename T>
    inline bool operator==(const Value &stateValue, const T& value) {
        return T(stateValue) == value;
    }
    template <typename T>
    inline bool operator==(const T& value, const Value &stateValue) {
        return T(stateValue) == value;
    }
            
    template <typename T>
    inline bool operator!=(const Value &stateValue, const T& value) {
        return T(stateValue) != value;
    }
    template <typename T>
    inline bool operator!=(const T& value, const Value &stateValue) {
        return T(stateValue) != value;
    }
    
    template <typename T>
    inline bool operator<(const Value &stateValue, const T& value) {
        return T(stateValue) < value;
    }
    template <typename T>
    inline bool operator<(const T& value, const Value &stateValue) {
        return T(stateValue) < value;
    }
            
    template <typename T>
    inline bool operator<=(const Value &stateValue, const T& value) {
        return T(stateValue) <= value;
    }
    template <typename T>
    inline bool operator<=(const T& value, const Value &stateValue) {
        return T(stateValue) <= value;
    }
            
    template <typename T>
    inline bool operator>(const Value &stateValue, const T& value) {
        return T(stateValue) > value;
    }
    template <typename T>
    inline bool operator>(const T& value, const Value &stateValue) {
        return T(stateValue) > value;
    }
    
    template <typename T>
    inline bool operator>=(const Value &stateValue, const T& value) {
        return T(stateValue) >= value;
    }
    template <typename T>
    inline bool operator>=(const T& value, const Value &stateValue) {
        return T(stateValue) >= value;
    }
    
    
    
    
    inline bool operator==(const Value &stateValue, const std::string& value) {
        return lua::String(stateValue) == value;
    }
    inline bool operator==(const std::string& value, const Value &stateValue) {
        return lua::String(stateValue) == value;
    }
    
    inline bool operator!=(const Value &stateValue, const std::string& value) {
        return lua::String(stateValue) != value;
    }
    inline bool operator!=(const std::string& value, const Value &stateValue) {
        return lua::String(stateValue) != value;
    }
    
    inline bool operator<(const Value &stateValue, const std::string& value) {
        return lua::String(stateValue) < value;
    }
    inline bool operator<(const std::string& value, const Value &stateValue) {
        return lua::String(stateValue) < value;
    }
    
    inline bool operator<=(const Value &stateValue, const std::string& value) {
        return lua::String(stateValue) <= value;
    }
    inline bool operator<=(const std::string& value, const Value &stateValue) {
        return lua::String(stateValue) <= value;
    }
    
    inline bool operator>(const Value &stateValue, const std::string& value) {
        return lua::String(stateValue) > value;
    }
    inline bool operator>(const std::string& value, const Value &stateValue) {
        return lua::String(stateValue) > value;
    }
    
    inline bool operator>=(const Value &stateValue, const std::string& value) {
        return lua::String(stateValue) >= value;
    }
    inline bool operator>=(const std::string& value, const Value &stateValue) {
        return lua::String(stateValue) >= value;
    }

    
    namespace stack {
        
        template<>
        inline bool check<lua::Value>(lua_State* luaState, int index)
        {
            return true;
        }
        
        /// Values direct forwarding
        template<>
        inline int push(lua_State* luaState, lua::Value value) {
            lua_pushvalue(luaState, value.getStackIndex());
            return 1;
        }
        
    }
}

