//
//  LuaFunction.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files

#pragma once

namespace lua {
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    class Function
    {
        std::shared_ptr<lua_State> _luaState;
        int _numArgs;
        mutable bool _executed;
        bool _protectedCall;
        
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
        
    public:
        // constructors
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        Function(const std::shared_ptr<lua_State>& luaState, int numArgs, bool protectedCall)
        : _luaState(luaState)
        , _numArgs(numArgs)
        , _executed(false)
        , _protectedCall(protectedCall) {}
        
        ~Function() {
            if (!_executed)
                callFunction(0);
        }
        
        // operator overloads
        //////////////////////////////////////////////////////////////////////////////////////////////////

        template<typename T>
        operator T() const {
            callFunction(1);
            
            auto retValue = stack::read<T>(_luaState.get(), -1);
            stack::pop(_luaState.get(), 1);
            return retValue;
        }
        
        // other functions
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
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