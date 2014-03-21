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
            Global,
        };

        Type _type;
        
        mutable bool _isGet;
        
    public:
        
        Key(LuaType::String name)
        : _string(name)
        , _type(Type::Global)
        , _isGet(false) {}
        
        void set(LuaType::Integer integer) {
            _type = Type::Integer;
            _integer = integer;
            _isGet = false;
        }
        
        void set(LuaType::String string) {
            _type = Type::String;
            _string = string;
            _isGet = false;
        }
        
        void push(lua_State* luaState) const {
            switch (_type) {
                    
                case Type::Integer:
                    printf("SET  %s\n", _string);
                    lua_pushinteger(luaState, _integer);
                    break;
                    
                case Type::Global:
                case Type::String:
                    printf("SET  %s\n", _string);
                    lua_pushstring(luaState, _string);
                    break;
            }
        }
        
        bool get(lua_State* luaState) const {
            
            if (!_isGet) {
                _isGet = true;
            
                switch (_type) {
                        
                    case Type::Global:
                        printf("GET  %s\n", _string);
                        lua_getglobal(luaState, _string);
                        break;
                        
                    case Type::Integer:
                        printf("GET  %d\n", _integer);
                        lua_rawgeti(luaState, -1, _integer);
                        break;
                        
                    case Type::String:
                        printf("GET  %s\n", _string);
                        lua_getfield(luaState, -1, _string);
                        break;
                }
                return true;
            }
            return false;
        }
    };

}