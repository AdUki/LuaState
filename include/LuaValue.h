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
                if (lua_pcall(_stack->state, sizeof...(Ts), LUA_MULTRET, 0)) {
                    const char* error = lua_tostring(_stack->state, -1);
                    lua_pop(_stack->state, 1);
                    throw RuntimeError(error);
                }
            }
            else
                lua_call(_stack->state, sizeof...(Ts), LUA_MULTRET);
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
        
        template<typename ... Ts>
        Value executeFunction(bool protectedCall, Ts... args) const & {
            
            int stackTop = stack::top(_stack->state);

            // We will duplicate Lua function value, because it will get poped from stack
            lua_pushvalue(_stack->state, _stack->top + _stack->pushed - _stack->grouped);
            
            callFunction(protectedCall, args...);
            int returnedValues = stack::top(_stack->state) - stackTop;
            
            LUASTATE_ASSERT(returnedValues >= 0);
            
            return Value(std::make_shared<detail::StackItem>(_stack->state, _stack->deallocQueue, stackTop, returnedValues, returnedValues == 0 ? 0 : returnedValues - 1));
        }
        
        /// We will check reference count and upon deletion we will restore stack to original value or push values to priority queue
        inline void deleteValue() {
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
        
        ~Value() {}

        Value(Value&& value) = default;
        Value(const Value& value) = default;
        
        Value& operator= (Value && value) && = default;
        Value& operator= (const Value& value) = default;

        /// With this function we will create lua::Ref instance
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename T>
        Value operator[](T key) const {
            stack::get(_stack->state, _stack->top + _stack->pushed - _stack->grouped, key);
            return Value(std::make_shared<detail::StackItem>(_stack->state, _stack->deallocQueue, stack::top(_stack->state) - 1, 1, 0));;
        }
        
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
        
        /// Call given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename ... Ts>
        Value operator()(Ts... args) const & {
            return executeFunction(false, args...);
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
            return stack::read<T>(_stack->state, _stack->top + _stack->pushed - _stack->grouped);
        }

        /// Set values to table to the given key.
        ///
        /// @param key      Key to which value will be stored
        /// @param value    Value to be stored to table on given key
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename K, typename T>
        void set(const K& key, const T& value) const {
            stack::push(_stack->state, key);
            stack::push(_stack->state, value);
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
        
        /// Will get pointer casted to given template type
        ///
        /// @return Pointer staticaly casted to given template type
        template <typename T>
        T* getPtr() const {
            return static_cast<T*>(Pointer(*this));
        }
            
        /// @returns Value position on stack
        int getStackIndex() const {
            assert(_stack->pushed > 0);
            return _stack->top + 1;
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
            
    namespace stack {
        
        /// Values direct forwarding
        template<>
        inline int push(lua_State* luaState, lua::Value value) {
            lua_pushvalue(luaState, value.getStackIndex());
            return 1;
        }
        
    }
}

