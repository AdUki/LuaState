//
//  LuaException.h
//  LuaState
//
//  Created by Simon Mikuda on 17/03/14.
//
//  See LICENSE and README.md files

#pragma once

#include <exception>

namespace lua {
    
    namespace stack {
        
        inline void dump (lua_State *L) {
            int i;
            int top = lua_gettop(L);
            for (i = 1; i <= top; i++) {  /* repeat for each level */
                int t = lua_type(L, i);
                switch (t) {
                    case LUA_TSTRING:  /* strings */
                        printf("`%s'", lua_tostring(L, i));
                        break;
                        
                    case LUA_TBOOLEAN:  /* booleans */
                        printf(lua_toboolean(L, i) ? "true" : "false");
                        break;
                        
                    case LUA_TNUMBER:  /* numbers */
                        printf("%g", lua_tonumber(L, i));
                        break;
                        
                    default:  /* other values */
                        printf("%s", lua_typename(L, t));
                        break;
                        
                }
                printf("  ");  /* put a separator */
            }
            printf("\n");  /* end the listing */
        }
        
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    class LoadError: public std::exception
    {
        std::string _message;
        
    public:
        LoadError(lua_State* luaState)
        : _message(lua_tostring(luaState, -1)) { lua_pop(luaState, 1); }
        
        virtual ~LoadError() throw() {}
        virtual const char* what() const throw() { return _message.c_str(); }
    };
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    class RuntimeError: public std::exception
    {
        std::string _message;
        
    public:
        RuntimeError(lua_State* luaState)
        : _message(lua_tostring(luaState, -1)) { lua_pop(luaState, 1); }
        
        virtual ~RuntimeError() throw() {}
        virtual const char* what() const throw() { return _message.c_str(); }
    };

    //////////////////////////////////////////////////////////////////////////////////////////////
    class TypeMismatchError : public std::exception
    {
        int _stackIndex;
        char _message[255];
        
    public:
        TypeMismatchError(lua_State* luaState, int index)
        : _stackIndex(index) {  }
        
        virtual ~TypeMismatchError() throw() { }
        virtual const char* what() const throw() {
            sprintf((char *)_message, "Type mismatch error at index %d.", _stackIndex);
            return _message;
        }
    };
}