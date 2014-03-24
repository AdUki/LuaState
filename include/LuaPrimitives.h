//
//  LuaPrimitives.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files

#pragma once

namespace lua
{
    struct Table {};
    
    typedef lua_Number Number;
    
    typedef int Integer;
    
    typedef bool Boolean;
    
    typedef const char* String;
    
    typedef std::nullptr_t Null;
    
    typedef void* Pointer;
}
