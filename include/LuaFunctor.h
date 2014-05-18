//
//  LuaFunctor.h
//  LuaState
//
//  Created by Simon Mikuda on 22/03/14.
//
//  See LICENSE and README.md files
//

#pragma once

namespace lua {
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// Base functor class with call function. It is used for registering lamdas, or regular functions
    struct BaseFunctor
    {
        BaseFunctor(const std::shared_ptr<lua_State>& luaState) {
            LUASTATE_DEBUG_LOG("Functor %p created!\n", this);
        }
        virtual ~BaseFunctor() {
            LUASTATE_DEBUG_LOG("Functor %p destructed!\n", this);
        }
        
        virtual int call(const std::shared_ptr<lua_State>& luaState, detail::DeallocQueue* deallocQueue) = 0;
    };
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// Functor with return values
    template <typename Ret, typename ... Args>
    struct Functor : public BaseFunctor {
        std::function<Ret(Args...)> function;
        
        Functor(const std::shared_ptr<lua_State>& luaState, std::function<Ret(Args...)> function)
        : BaseFunctor(luaState)
        , function(function){
        }
        
        int call(const std::shared_ptr<lua_State>& luaState, detail::DeallocQueue* deallocQueue) {
            Ret value = traits::apply(function, stack::get_and_pop<Args...>(luaState, deallocQueue, stack::top(luaState) - 1));
            return stack::push(luaState, value);
        }
    };
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    /// Functor with no return values
    template <typename ... Args>
    struct Functor<void, Args...> : public BaseFunctor {
        std::function<void(Args...)> function;
        
        Functor(const std::shared_ptr<lua_State>& luaState, std::function<void(Args...)> function)
        : BaseFunctor(luaState)
        , function(function) {}
        
        int call(const std::shared_ptr<lua_State>& luaState, detail::DeallocQueue* deallocQueue) {
            traits::apply(function, stack::get_and_pop<Args...>(luaState, deallocQueue, stack::top(luaState)));
            return 0;
        }
    };
    
    namespace stack {
        
        template <typename Ret, typename ... Args>
        inline int push(const std::shared_ptr<lua_State>& luaState, Ret(*function)(Args...)) {
            BaseFunctor** udata = (BaseFunctor **)lua_newuserdata(luaState.get(), sizeof(BaseFunctor *));
            *udata = new Functor<Ret, Args...>(luaState, function);
            
            luaL_getmetatable(luaState.get(), "luaL_Functor");
            lua_setmetatable(luaState.get(), -2);
            return 1;
        }
        
        template <typename Ret, typename ... Args>
        inline int push(const std::shared_ptr<lua_State>& luaState, std::function<Ret(Args...)> function) {
            BaseFunctor** udata = (BaseFunctor **)lua_newuserdata(luaState.get(), sizeof(BaseFunctor *));
            *udata = new Functor<Ret, Args...>(luaState, function);
            
            luaL_getmetatable(luaState.get(), "luaL_Functor");
            lua_setmetatable(luaState.get(), -2);
            return 1;
        }
        
        template<typename T>
        inline int push(const std::shared_ptr<lua_State>& luaState, T function)
        {
            push(luaState, (typename traits::function_traits<T>::Function)(function));
            return 1;
        }
        
    }
}