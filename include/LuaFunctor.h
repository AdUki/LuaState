//
//  LuaStore.h
//  LuaState
//
//  Created by Simon Mikuda on 22/03/14.
//
//

#pragma once

#include "./Traits.h"
#include "./LuaStack.h"

namespace lua {
    
    struct BaseFunctor
    {
        BaseFunctor(lua_State* luaState, size_t size) {
            LUASTATE_DEBUG_LOG("Functor %p created!\n", this);
        }
        virtual ~BaseFunctor() {
            LUASTATE_DEBUG_LOG("Functor %p destructed!\n", this);
        }
        
        virtual int call(lua_State* luaState) = 0;
    };
    
    template <typename Ret, typename ... Args>
    struct Functor : public BaseFunctor {
        std::function<Ret(Args...)> function;
        
        Functor(lua_State* luaState, std::function<Ret(Args...)> function)
        : BaseFunctor(luaState, sizeof(*this))
        , function(function){
        }
        
        int call(lua_State* luaState) {
            Ret value = traits::apply(function, stack::get_and_pop<Args...>(luaState));
            return stack::push(luaState, value);
        }
    };
    
    template <typename ... Args>
    struct Functor<void, Args...> : public BaseFunctor {
        std::function<void(Args...)> function;
        
        Functor(lua_State* luaState, std::function<void(Args...)> function)
        : BaseFunctor(luaState, sizeof(*this))
        , function(function) {}
        
        int call(lua_State* luaState) {
            traits::apply(function, stack::get_and_pop<Args...>(luaState));
            return 0;
        }
    };
    
    namespace stack {
        
        template <typename Ret, typename ... Args>
        inline int push(lua_State* luaState, Ret(*function)(Args...)) {
            BaseFunctor** udata = (BaseFunctor **)lua_newuserdata(luaState, sizeof(BaseFunctor *));
            *udata = new Functor<Ret, Args...>(luaState, function);
            
            luaL_getmetatable(luaState, "luaL_Functor");
            lua_setmetatable(luaState, -2);
            return 1;
        }
        
        template <typename Ret, typename ... Args>
        inline int push(lua_State* luaState, std::function<Ret(Args...)> function) {
            BaseFunctor** udata = (BaseFunctor **)lua_newuserdata(luaState, sizeof(BaseFunctor *));
            *udata = new Functor<Ret, Args...>(luaState, function);
            
            luaL_getmetatable(luaState, "luaL_Functor");
            lua_setmetatable(luaState, -2);
            return 1;
        }
        
        template<typename T>
        inline int push(lua_State* luaState, T function)
        {
            push(luaState, (typename traits::function_traits<T>::Function)(function));
            return 1;
        }
        
    }
}