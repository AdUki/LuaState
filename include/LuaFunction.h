//
//  LuaFunction.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files

#pragma once

namespace lua {
    
    class Value;
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// This class is for calling lua values and retrieving return values.
    class Function
    {
        friend class Value;
        
        /// Lua state shared pointer
        std::shared_ptr<lua_State> _luaState;
        
        /// Number of arguments
        int _numArgs;
        
        /// Indicates if it is protected call. Otherwise it is just regular call.
        bool _protectedCall;
        
        /// Indicates if function was executed. Needed for "automatic" calling while destructing object.
        mutable bool _executed;
        
        /// Call function.
        ///
        /// @pre  Arguments are pushed to stack.
        /// @post Function pushes return values to stack.
        ///
        /// @param numRet   Number of return values
        inline void callFunction(int numRet) const {
            _executed = true;
            if (_protectedCall) {
                if (lua_pcall(_luaState.get(), _numArgs, numRet, 0)) {
                    const char* error = lua_tostring(_luaState.get(), -1);
                    lua_pop(_luaState.get(), 1);
                    throw RuntimeError(error);
                }
            }
            else {
                lua_call(_luaState.get(), _numArgs, numRet);
            }
        }
        
        /// Constructs instance with specified number of arguments and type of call.
        ///
        /// @pre  Arguments are pushed to stack.
        ///
        /// @param luaState         Shared pointer of Lua state
        /// @param numArgs          Number of arguments of function
        /// @param protectedCall
        Function(const std::shared_ptr<lua_State>& luaState, int numArgs, bool protectedCall)
        : _luaState(luaState)
        , _numArgs(numArgs)
        , _protectedCall(protectedCall)
        , _executed(false)
        {}
        
    public:
        
        /// When function was not executed manualy it is executed in desctuctor.
        ///
        /// @attention While making protected call we cannot throw exceptions. Please execute function manualy with .execute() function
        ~Function() {
            if (!_executed) {
                
                // Use .execute() function while protected call.
                assert(!_protectedCall);
                
                callFunction(0);
            }
        }

        /// Cast operator.
        ///
        /// @return Single or multiple (using std::tuple) return values
        template<typename T>
        operator T() const {
            callFunction(1);
            
            auto retValue = stack::read<T>(_luaState.get(), -1);
            stack::pop(_luaState.get(), 1);
            return retValue;
        }
        
        /// Executes function. It is not necessary when we are using regular calls.
        ///
        /// @return Single or multiple (using std::tuple) return values
        template <typename ... Ret>
        std::tuple<Ret...> execute() const {
            callFunction(sizeof...(Ret));
            auto returnValue = stack::get_and_pop<Ret...>(_luaState.get());
            return returnValue;
        }
    };
    
    // compare operators
    //////////////////////////////////////////////////////////////////////////////////////////////////
    inline bool operator==(const Function &value, const char *string)        { return strcmp(value, string) == 0; }
    inline bool operator==(const char *string, const Function &value)        { return strcmp(value, string) == 0; }
    inline bool operator==(const Function &value, const std::string& string) { return strcmp(value, string.c_str()) == 0; }
    inline bool operator==(const std::string& string, const Function &value) { return strcmp(value, string.c_str()) == 0; }
    
    template <typename T>
    inline bool operator==(const Function &stateValue, const T& value) {
        return T(stateValue) == value;
    }
    template <typename T>
    inline bool operator==(const T& value, const Function &stateValue) {
        return T(stateValue) == value;
    }
}