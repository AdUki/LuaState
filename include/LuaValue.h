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
        detail::DeallocQueue* _deallocQueue = nullptr;
        
        /// Indicates number of pushed values to stack on lua::Value when created
        int _stackTop;
        
        /// Indicates number pushed values which were pushed by this lua::Value instance
        mutable int _pushedValues;
        
        mutable int _groupedValues;

        /// Constructor for creating lua::Ref instances
        ///
        /// @param luaState     Shared pointer of Lua state
        Value(const std::shared_ptr<lua_State>& luaState, detail::DeallocQueue* deallocQueue)
        : _luaState(luaState)
        , _deallocQueue(deallocQueue)
        , _stackTop(stack::top(_luaState))
        , _pushedValues(0)
        , _groupedValues(0)
        {
        }
        
        /// Constructor for lua::State class. Whill get global in _G table with name
        ///
        /// @param luaState     Shared pointer of Lua state
        /// @param name         Key of global value
        Value(const std::shared_ptr<lua_State>& luaState, detail::DeallocQueue* deallocQueue, const char* name)
        : Value(luaState, deallocQueue)
        {
            stack::get_global(_luaState, name);
            ++_pushedValues;
        }
        
        template<typename ... Ts>
        int callFunction(bool protectedCall, Ts... args) const {
            
            stack::push(_luaState, args...);
            if (protectedCall) {
                if (lua_pcall(_luaState.get(), sizeof...(Ts), LUA_MULTRET, 0)) {
                    const char* error = lua_tostring(_luaState.get(), -1);
                    lua_pop(_luaState.get(), 1);
                    throw RuntimeError(error);
                }
            }
            else
                lua_call(_luaState.get(), sizeof...(Ts), LUA_MULTRET);
            
            return stack::top(_luaState) - (_stackTop + _pushedValues - _groupedValues);
        }

        template<typename ... Ts>
        Value&& executeFunction(bool protectedCall, Ts... args) {
            _groupedValues = callFunction(protectedCall, args...);
            _pushedValues += _groupedValues;
            return std::move(*this);
        }
        
        template<typename ... Ts>
        Value executeFunction(bool protectedCall, Ts... args) const & {
            Value value(_luaState, _deallocQueue);
            int returnedValues = _stackTop + _pushedValues - stack::top(_luaState);
            
            // We will duplicate Lua function value, because it will get poped from stack
            lua_pushvalue(_luaState.get(), _stackTop + _pushedValues - _groupedValues);
            
            returnedValues += callFunction(protectedCall, args...);
            value._groupedValues = returnedValues <= 0 ? 0 : returnedValues - 1;
            value._pushedValues += returnedValues;
            
            return std::move(value);
        }
        
    public:
        
        /// Enable to initialize empty Value, so we can set it up later
        Value() : _luaState(nullptr) {}

        /// Constructor for returning values from functions
        Value(const std::shared_ptr<lua_State>& luaState, detail::DeallocQueue* deallocQueue, int index)
        : Value(luaState, deallocQueue)
        {
            _stackTop = index - 1;
            _pushedValues = 1;
        }
        
        /// Upon deletion we will restore stack to original value
        ~Value() {
            if (_luaState != nullptr) {

                // Check because of exceptions during function calls
                int currentStackTop = stack::top(_luaState);
                if (_pushedValues == 0 || currentStackTop == 0 || currentStackTop - _stackTop < _pushedValues)
                    return;
                
                // We will check if we haven't pushed some other new lua::Value to stack
                if (_stackTop + _pushedValues == currentStackTop) {
                    
                    // We will check deallocation priority queue, if there are some lua::Value instances to be deleted
                    while (!_deallocQueue->empty() && _deallocQueue->top().stackCap == _stackTop) {
                        _stackTop -= _deallocQueue->top().numElements;
                        _deallocQueue->pop();
                    }
                    stack::settop(_luaState, _stackTop);
                }
                // If yes we can't pop values, we must pop it after deletion of newly created lua::Value
                // We will put this deallocation to our priority queue, so it will be deleted as soon as possible
                else
                    _deallocQueue->push(detail::DeallocStackItem(_stackTop, _pushedValues));
            }
        }
        
        // Default move constructor
        Value(Value&&) = default;
        
        // In assigment we swap values, so initialized values will be properly released
        Value& operator= (Value && value) {
            std::swap(_luaState, value._luaState);
            std::swap(_pushedValues, value._pushedValues);
            std::swap(_stackTop, value._stackTop);
            std::swap(_deallocQueue, value._deallocQueue);
            std::swap(_groupedValues, value._groupedValues);
            
            return *this;
        }

        // Default copy constructor, because gcc-4.7 compiler use it with std::make_tuple
        // TODO: if later compilers will use move, delete it
        Value(const Value& value) = default;
            
        // Deleted copy operator
        Value& operator= (Value& value) = delete;

        /// With this function we will create lua::Ref instance
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename T>
        Value operator[](T key) const {
            Value value(_luaState, _deallocQueue);
            stack::get(_luaState, _stackTop + _pushedValues - _groupedValues, key);
            value._pushedValues = 1;
            
            return std::move(value);
        }
        
        /// While chaining [] operators we will call this function multiple times and can query nested tables.
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename T>
        Value&& operator[](T key) && {
            stack::get(_luaState, _stackTop + _pushedValues - _groupedValues, key);
            ++_pushedValues;
            
            _groupedValues = 0;
            
            return std::move(*this);
        }
        
        /// Call given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename ... Ts>
        Value operator()(Ts... args) const & {
            return std::move(executeFunction(false, args...));
        }
        
        /// Call given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename ... Ts>
        Value&& operator()(Ts... args) && {
            return std::forward<Value>(executeFunction(false, args...));
        }
        
        /// Protected call of given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename ... Ts>
        Value call(Ts... args) const & {
            return executeFunction(true, args...);
        }
        
        /// Protected call of given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename ... Ts>
        Value&& call(Ts... args) && {
            return executeFunction(true, args...);
        }
        
        /// Cast operator. Enables to pop values from stack and store it to variables
        ///
        /// @return Any value of type from LuaPrimitives.h
        template<typename T>
        operator T() const {
            return stack::read<T>(_luaState, _stackTop + _pushedValues - _groupedValues);
        }

        /// Set values to table to the given key.
        ///
        /// @param key      Key to which value will be stored
        /// @param value    Value to be stored to table on given key
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename K, typename T>
        void set(const K& key, const T& value) const {
            stack::push(_luaState, key);
            stack::push(_luaState, value);
            lua_settable(_luaState.get(), _stackTop + _pushedValues - _groupedValues);
        }

        /// Check if queryied value is some type from LuaPrimitives.h file
        ///
        /// @return true if yes false if no
        template <typename T>
        bool is() const {
            return stack::check<T>(_luaState, _stackTop + _pushedValues - _groupedValues);
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
                value = stack::read<T>(_luaState, _stackTop + _pushedValues - _groupedValues);
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
}

