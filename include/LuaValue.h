//
//  LuaValue.h
//  LuaState
//
//  Created by Simon Mikuda on 17/03/14.
//
//  See LICENSE and README.md files

#pragma once

#ifdef LUASTATE_DEBUG_MODE
static int REF_COUNTER = 0;
#   define LUASTATE_ADD_REF_COUNT() REF_COUNTER++
#   define LUASTATE_REM_REF_COUNT() REF_COUNTER--
#else
#   define LUASTATE_ADD_REF_COUNT()
#   define LUASTATE_REM_REF_COUNT()
#endif

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
        
        struct StackState {
            
            lua_State* state;
            detail::DeallocQueue* deallocQueue = nullptr;
            
            /// Indicates number of pushed values to stack on lua::Value when created
            int top;
            
            /// Indicates number pushed values which were pushed by this lua::Value instance
            mutable int pushed;
            
            /// Indicates multi returned values, because the we want first returned value and not the last
            int grouped;
            
            /// Reference counting
            int refCounter;
            
            StackState() : state(nullptr), deallocQueue(nullptr)
            {
                LUASTATE_ADD_REF_COUNT();
            }
            
            StackState(lua_State* luaState, detail::DeallocQueue* deallocQueue, int stackTop, int pushedValues, int groupedValues, int refCounter)
            : state(luaState)
            , deallocQueue(deallocQueue)
            , top(stackTop)
            , pushed(pushedValues)
            , grouped(groupedValues)
            , refCounter(refCounter)
            {
                LUASTATE_ADD_REF_COUNT();
            }
            
            ~StackState()
            {
                LUASTATE_REM_REF_COUNT();
            }
        };
        
        StackState* _stack;
        
        /// Constructor for creating lua::Ref instances
        ///
        /// @param luaState     Pointer of Lua state
        /// @param deallocQueue Queue for deletion values initialized from given luaState
        Value(lua_State* luaState, detail::DeallocQueue* deallocQueue)
        : _stack(new StackState(luaState, deallocQueue, stack::top(luaState), 0, 0, 0))
        {
        }
        
        /// Constructor for lua::State class. Whill get global in _G table with name
        ///
        /// @param luaState     Pointer of Lua state
        /// @param deallocQueue Queue for deletion values initialized from given luaState
        /// @param name         Key of global value
        Value(lua_State* luaState, detail::DeallocQueue* deallocQueue, const char* name)
        : _stack(new StackState(luaState, deallocQueue, stack::top(luaState), 1, 0, 1))
        {
            stack::get_global(_stack->state, name);
        }
        
        template<typename ... Ts>
        int callFunction(bool protectedCall, Ts... args) const {
            
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
            
            return stack::top(_stack->state) - (_stack->top + _stack->pushed - _stack->grouped);
        }

        template<typename ... Ts>
        Value&& executeFunction(bool protectedCall, Ts... args) {
            
            // we check if there are not pushed values before function
            if (_stack->top + _stack->pushed < stack::top(_stack->state)) {
                
                _stack->deallocQueue->push(detail::DeallocStackItem(_stack->top, _stack->pushed));

                lua_pushvalue(_stack->state, _stack->top + 1);
                
                _stack->top = stack::top(_stack->state) - 1;
                _stack->pushed = 1;
                _stack->grouped = 0;
            }
            
            // Function must be on top of stack
            LUASTATE_ASSERT(_stack->top + _stack->pushed == stack::top(_stack->state));
            LUASTATE_ASSERT(stack::check<Callable>(_stack->state, stack::top(_stack->state)));
            
            _stack->grouped = callFunction(protectedCall, args...);
            _stack->pushed += _stack->grouped;
            
            return std::move(*this);
        }
        
        template<typename ... Ts>
        Value executeFunction(bool protectedCall, Ts... args) const & {
            
            Value value(_stack->state, _stack->deallocQueue);
            int returnedValues = _stack->top + _stack->pushed - stack::top(_stack->state);
            
            // We will duplicate Lua function value, because it will get poped from stack
            lua_pushvalue(_stack->state, _stack->top + _stack->pushed - _stack->grouped);
            
            // Function must be on top of stack
            LUASTATE_ASSERT(stack::check<Callable>(_stack->state, stack::top(_stack->state)));
            
            returnedValues += callFunction(protectedCall, args...);
            value._stack->grouped = returnedValues == 0 ? 0 : returnedValues - 1;
            value._stack->pushed += returnedValues;
            
            LUASTATE_ASSERT(_stack->grouped >= 0);
            return value;
        }
        
        /// We will check reference count and upon deletion we will restore stack to original value or push values to priority queue
        inline void deleteValue() {
            
            // Check if there is value assigned
            if (_stack == nullptr)
                return;
            
            // Check reference counter
            _stack->refCounter = _stack->refCounter - 1;
            if (_stack->refCounter > 0)
                return;
            else
                delete _stack;
            
            // Check if stack is managed automaticaly (_deallocQueue == nullptr), which is when we call C functions from Lua
            if (_stack->deallocQueue != nullptr) {
            
                // Check if we dont try to release same values twice
                int currentStackTop = stack::top(_stack->state);
                if (currentStackTop < _stack->pushed + _stack->top) {
                    stack::dump(_stack->state);
                    return;
                }
                
                // We will check if we haven't pushed some other new lua::Value to stack
                if (_stack->top + _stack->pushed == currentStackTop) {
                    
                    // We will check deallocation priority queue, if there are some lua::Value instances to be deleted
                    while (!_stack->deallocQueue->empty() && _stack->deallocQueue->top().stackCap == _stack->top) {
                        _stack->top -= _stack->deallocQueue->top().numElements;
                        _stack->deallocQueue->pop();
                    }
                    stack::settop(_stack->state, _stack->top);
                }
                // If yes we can't pop values, we must pop it after deletion of newly created lua::Value
                // We will put this deallocation to our priority queue, so it will be deleted as soon as possible
                else
                    _stack->deallocQueue->push(detail::DeallocStackItem(_stack->top, _stack->pushed));
            }
        }
        
    public:
        
        /// Enable to initialize empty Value, so we can set it up later
        Value() : _stack(nullptr) {
        }
            
        /// Constructor for returning values from functions
        ///
        /// @param luaState     Pointer of Lua state
        /// @param deallocQueue Queue for deletion values initialized from given luaState
        /// @param index        Index of value which is already in stack
        Value(lua_State* luaState, detail::DeallocQueue* deallocQueue, int index)
        {
            if (stack::top(luaState) < index)
                // We request more values, that are returned
                _stack = new StackState(luaState, nullptr, 0, 0, 0, 1);
            else
                _stack = new StackState(luaState, deallocQueue, index, 1, 0, 1);
        }
        
        ~Value() {
            deleteValue();
        }
        
        /// Move constructor
        Value(Value&& value) {
            _stack = value._stack;
            value._stack = nullptr;
        }

        /// Move assingment
        Value& operator= (Value && value) && {
            _stack = value._stack;
            value._stack = nullptr;
            return *this;
        }

        /// Copy assignment
        Value& operator= (const Value& value) {
            
            if (value._stack == nullptr)
                _stack = nullptr;
            
            else if (_stack == nullptr || _stack->top != value._stack->top) {
                
                deleteValue();
                
                _stack = value._stack;
                _stack->refCounter = _stack->refCounter + 1;
            }
            
            return *this;
        }

        /// Some compilers use copy constructor with std::make_tuple. We will use reference counter to correctly deallocate values from stack
        Value(const Value& value) {
            _stack = value._stack;
            
            if (_stack != nullptr)
                _stack->refCounter = _stack->refCounter + 1;
        }

        /// With this function we will create lua::Ref instance
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename T>
        Value operator[](T key) const {
            Value value(_stack->state, _stack->deallocQueue);
            stack::get(_stack->state, _stack->top + _stack->pushed - _stack->grouped, key);
            value._stack->pushed = 1;
            
            return value;
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

