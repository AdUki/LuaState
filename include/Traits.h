//
//  Lambda.h
//  LuaState
//
//  Created by Simon Mikuda on 22/03/14.
//
//

#pragma once

namespace lua {
    
    template <typename T>
    struct function_traits
    : public function_traits<decltype(&T::operator())>
    {};
    
    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits<ReturnType(ClassType::*)(Args...) const>
    {
        enum { arity = sizeof...(Args) };
        
        typedef ReturnType ResultType;
        typedef std::function<ReturnType(Args...)> Function;
        
        template <size_t i>
        struct Arg {
            typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
        };
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    template<int...> struct index_tuple{};
    
    template<int I, typename IndexTuple, typename ... Types>
    struct make_indexes_impl;
    
    template<int I, int... Indexes, typename T, typename ... Types>
    struct make_indexes_impl<I, index_tuple<Indexes...>, T, Types...>
    {
        typedef typename make_indexes_impl<I + 1, index_tuple<Indexes..., I>, Types...>::type type;
    };
    
    template<int I, int... Indexes>
    struct make_indexes_impl<I, index_tuple<Indexes...> >
    {
        typedef index_tuple<Indexes...> type;
    };
    
    template<typename ... Types>
    struct make_indexes : make_indexes_impl<0, index_tuple<>, Types...>
    {};
    
    template<class Ret, class... Args, int... Indexes >
    Ret apply_helper(std::function<Ret(Args...)> pf, index_tuple< Indexes... >, std::tuple<Args...>&& tup)
    {
        return pf( std::forward<Args>( std::get<Indexes>(tup))... );
    }
    
    template<class Ret, class ... Args>
    Ret apply(std::function<Ret(Args...)> pf, const std::tuple<Args...>&  tup)
    {
        return apply_helper(pf, typename make_indexes<Args...>::type(), std::tuple<Args...>(tup));
    }
    
    template<class Ret, class ... Args>
    Ret apply(std::function<Ret(Args...)> pf, std::tuple<Args...>&&  tup)
    {
        return apply_helper(pf, typename make_indexes<Args...>::type(), std::forward<std::tuple<Args...>>(tup));
    }
}