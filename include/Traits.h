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
    struct lambda_traits : public lambda_traits<decltype(&T::operator())> {};

    template <typename T, typename Ret, typename ... Args>
    struct lambda_traits<Ret(T::*)(Args...) const> {
        typedef std::function<Ret(Args...)> Fun;
    };
    
    template<int ...>
    struct seq { };
    
    template<int N, int ...S>
    struct gens : gens<N-1, N-1, S...> { };
    
    template<int ...S>
    struct gens<0, S...> {
        typedef seq<S...> type;
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