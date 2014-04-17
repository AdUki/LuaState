//
//  LuaReturn.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files

#pragma once

#include "./LuaFunction.h"

namespace lua {
    
    /// Class for automaticly cas lua::Function instance to multiple return values with lua::tie
	template <typename ... Ts>
	class Return
    {
        /// Return values
	    std::tuple<Ts&...> _tuple;
        
	public:
        
        /// Constructs class with given arguments
        ///
        /// @param args    Return values
	    Return(Ts&... args)
        : _tuple(args...) {}
        
        /// Operator sets values to std::tuple
        ///
        /// @param function     Function being called
	    void operator= (const Function& function) {
            _tuple = function.execute<typename std::remove_reference<Ts>::type...>();
	    }

	};
    
    /// Use this function when you want to retrieve multiple return values from lua::Function
    template <typename ... Ts>
    Return<Ts&...> tie(Ts&... args) {
        return Return<Ts&...>(args...);
    }
}
