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
            printf("Functor %p created!\n", this);
        }
        
        virtual ~BaseFunctor() {
            printf("Functor %p destructed!\n", this);
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
            Ret value = apply(function, stack::get_and_pop<Args...>(luaState));
            stack::push(luaState, value);
            return 1; // TODO: enable return tuple and push more values
        }
    };
    
    template <typename ... Args>
    struct Functor<void, Args...> : public BaseFunctor {
        std::function<void(Args...)> function;
        
        Functor(lua_State* luaState, std::function<void(Args...)> function)
        : BaseFunctor(luaState, sizeof(*this))
        , function(function) {}
        
        int call(lua_State* luaState) {
            apply(function, stack::get_and_pop<Args...>(luaState));
            return 0;
        }
    };
    
    namespace stack {
        
        template <typename Ret, typename ... Args>
        void push(lua_State* luaState, Ret(*function)(Args...)) {
            BaseFunctor** udata = (BaseFunctor **)lua_newuserdata(luaState, sizeof(BaseFunctor *));
            *udata = new Functor<Ret, Args...>(luaState, function);
            
            luaL_getmetatable(luaState, "luaL_Functor");
            lua_setmetatable(luaState, -2);
        }

        
        template <typename Ret, typename ... Args>
        void push(lua_State* luaState, std::function<Ret(Args...)> function) {
            BaseFunctor** udata = (BaseFunctor **)lua_newuserdata(luaState, sizeof(BaseFunctor *));
            *udata = new Functor<Ret, Args...>(luaState, function);
            
            luaL_getmetatable(luaState, "luaL_Functor");
            lua_setmetatable(luaState, -2);
        }
        
        template<typename T>
        inline void push(lua_State* luaState, T lambda)
        {
            push(luaState, (typename lambda_traits<T>::Fun)(lambda));
        }
    }
}