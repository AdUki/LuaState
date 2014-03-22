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
        
    public:
        // constructors
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        Function(const std::shared_ptr<lua_State>& luaState, int numArgs)
        : _luaState(luaState)
        , _numArgs(numArgs)
        , _executed(false) {}
        
        ~Function() {
            if (!_executed) {
                lua_call(_luaState.get(), _numArgs, 0);
            }
        }
        
        // operator overloads
        //////////////////////////////////////////////////////////////////////////////////////////////////

        template<typename T>
        operator T() const {
            lua_call(_luaState.get(), _numArgs, 1);
            _executed = true;
            
            auto retValue = stack::read<T>(_luaState.get(), -1);
            stack::pop(_luaState.get(), 1);
            return retValue;
        }
        
        // other functions
        //////////////////////////////////////////////////////////////////////////////////////////////////

        template <typename ... Ret>
        std::tuple<Ret...> call() const {
            _executed = true;
            constexpr size_t numRet = sizeof...(Ret);
            
            lua_call(_luaState.get(), _numArgs, numRet);
            stack::dump(_luaState.get());
            auto returnValue = stack::get_and_pop<Ret...>(_luaState.get());
            
            return returnValue;
        }
    };
}