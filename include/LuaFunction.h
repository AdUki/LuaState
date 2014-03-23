//
//  LuaFunction.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//

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
                printf("pcall\n");
                if (lua_pcall(_luaState.get(), _numArgs, numRet, 0)) {
                    const char* error = lua_tostring(_luaState.get(), -1);
                    lua_pop(_luaState.get(), 1);
                    throw RuntimeError(error);
                }
            }
            else {
                printf("call\n");
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
}