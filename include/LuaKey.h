//
//  LuaKey.h
//  LuaState
//
//  Created by Simon Mikuda on 19/03/14.
//
//

#pragma once

namespace lua {
    
    class Key
    {
        LuaType::Integer _integer;
        LuaType::String _string;
        
        enum class Type {
            Integer,
            String,
        };

        Type _type;
        
    public:
        
        void set(LuaType::Integer integer) {
            _type = Type::Integer;
            _integer = integer;
        }
        
        void set(LuaType::String string) {
            _type = Type::String;
            _string = string;
        }
        
        void push(lua_State* luaState) const {
            switch (_type) {
                case Type::Integer:
                    lua_pushinteger(luaState, _integer);
                    break;
                    
                case Type::String:
                    lua_pushstring(luaState, _string);
                    break;
            }
        }
    };

}