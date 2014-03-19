//
//  LuaStack.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//

#pragma once

#include <lua.hpp>

#include "./LuaPrimitives.h"

namespace lua { namespace stack {
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    template <std::size_t... Is>
    struct Indexes {};
    
    template <std::size_t N, std::size_t... Is>
    struct IndexesBuilder : IndexesBuilder<N-1, N-1, Is...> {};
    
    template <std::size_t... Is>
    struct IndexesBuilder<0, Is...> {
        using index = Indexes<Is...>;
    };
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    inline int numberOfPushedValues(lua_State* luaState) {
        return lua_gettop(luaState);
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    template<typename T>
    inline void push(lua_State* luaState, T value);
    
    template<typename T, typename... Ts>
    inline void push(lua_State* luaState, const T value, const Ts... values) {
        push(luaState, value);
        push(luaState, values...);
    }

    template<>
    inline void push(lua_State* luaState, int value) {
        printf("  PUSH  %d\n", value);
        lua_pushinteger(luaState, value);
    }

    template<>
    inline void push(lua_State* luaState, LuaType::String value) {
        printf("  PUSH  %s\n", value);
        lua_pushstring(luaState, value);
    }

    template<>
    inline void push(lua_State* luaState, LuaType::Number value) {
        printf("  PUSH  %lf\n", value);
        lua_pushnumber(luaState, value);
    }

    template<>
    inline void push(lua_State* luaState, LuaType::Boolean value) {
        printf("  PUSH  %s\n", value ? "true" : "false");
        lua_pushboolean(luaState, value);
    }

    template<>
    inline void push(lua_State* luaState, LuaType::Null value) {
        printf("  PUSH  null\n");
        lua_pushnil(luaState);
    }

    template<>
    inline void push(lua_State* luaState, float value) {
        push<LuaType::Number>(luaState, value);
    }
    
    template<>
    inline void push(lua_State* luaState, const std::string& value) {
        push<LuaType::String>(luaState, value.c_str());
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    inline T read(lua_State* luaState, int index);

    template<>
    inline LuaType::Integer read(lua_State* luaState, int index) {
        return lua_tointeger(luaState, index);
    }

    template<>
    inline LuaType::String read(lua_State* luaState, int index) {
        return lua_tostring(luaState, index);;
    }

    template<>
    inline LuaType::Number read(lua_State* luaState, int index) {
        return lua_tonumber(luaState, index);
    }

    template<>
    inline LuaType::Boolean read(lua_State* luaState, int index) {
        return lua_toboolean(luaState, index);
    }

    template<>
    inline LuaType::Null read(lua_State* luaState, int index) {
        return nullptr;
    }

    template<>
    inline float read(lua_State* luaState, int index) {
        return read<LuaType::Number>(luaState, index);
    }
    
    template<>
    inline std::string read(lua_State* luaState, int index) {
        return read<LuaType::String>(luaState, index);
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    inline void pop(lua_State* luaState, int n) {
        printf("  POP  %d\n", n);
        lua_pop(luaState, n);
    }
    
    template<typename T>
    inline T pop_front(lua_State* luaState) {
        T value = read<T>(luaState, 1);
        lua_remove(luaState, 0);
        return value;
    }
    
    template<typename T>
    inline T pop_back(lua_State* luaState) {
        T value = read<T>(luaState, -1);
        pop(luaState, 1);
        return value;
    }
    
    template<std::size_t I, typename... Ts>
    class Pop {

        template<std::size_t... Is>
        static std::tuple<Ts...> create(lua_State* luaState, Indexes<Is...>) {
            return std::make_tuple(read<Ts>(luaState, Is + 1)...);
        }
        
    public:
        static std::tuple<Ts...> getTable(lua_State* luaState) {
            return create(luaState, typename IndexesBuilder<I>::index());
        }
    };
    
    template<typename... Ts>
    inline std::tuple<Ts...> pop(lua_State* luaState) {
        constexpr size_t num = sizeof...(Ts);
        auto value = Pop<num, Ts...>::getTable(luaState);
        stack::pop(luaState, num);
        return value;
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    template<typename T>
    inline void get(lua_State* luaState, int index, T key) {}
    
    template<>
    inline void get(lua_State* luaState, int index, const char* key) {
        printf("GET  %s\n", key);
        lua_getfield(luaState, index, key);
    }
    
    template<>
    inline void get(lua_State* luaState, int index, int key) {
        printf("GET  %d\n", key);
        lua_rawgeti(luaState, index, key);
    }
    
}}
