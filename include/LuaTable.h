//
//  LuaTable.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//

#pragma once

#include "./LuaFunction.h"

namespace lua {
    
	template <typename... Ts>
	class Table
    {
	    std::tuple<Ts&...> _tuple;
        
	public:
        
        // constructors
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
	    Table(Ts&... args)
        : _tuple(args...) {}
        
        // operator overloads
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
	    void operator= (const Function& function) {
            _tuple = function.call<typename std::remove_reference<Ts>::type...>();
	    }
        
        // other functions
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        // none
	};
    
    template <typename... Ts>
    Table<Ts&...> tie(Ts&... args) {
        return Table<Ts&...>(args...);
    }
}
