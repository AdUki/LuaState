//
//  LuaStack.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files

#pragma once

#include "./Traits.h"
#include "./LuaPrimitives.h"

#include <cmath>

namespace lua { namespace stack {
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    inline int numberOfPushedValues(lua_State* luaState) {
        return lua_gettop(luaState);
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////

    inline int push(lua_State* luaState) { return 0; }
    
    template<typename T>
    inline int push(lua_State* luaState, T value);
    
    template<typename T, typename ... Ts>
    inline int push(lua_State* luaState, const T value, const Ts... values) {
        push(luaState, value);
        push(luaState, values...);
        return sizeof...(Ts) + 1;
    }

    template<>
    inline int push(lua_State* luaState, int value) {
        LUASTATE_DEBUG_LOG("  PUSH  %d\n", value);
        lua_pushinteger(luaState, value);
        return 1;
    }

    template<>
    inline int push(lua_State* luaState, lua::String value) {
        LUASTATE_DEBUG_LOG("  PUSH  %s\n", value);
        lua_pushstring(luaState, value);
        return 1;
    }

    template<>
    inline int push(lua_State* luaState, lua::Number value) {
        LUASTATE_DEBUG_LOG("  PUSH  %lf\n", value);
        lua_pushnumber(luaState, value);
        return 1;
    }

    template<>
    inline int push(lua_State* luaState, lua::Boolean value) {
        LUASTATE_DEBUG_LOG("  PUSH  %s\n", value ? "true" : "false");
        lua_pushboolean(luaState, value);
        return 1;
    }

    template<>
    inline int push(lua_State* luaState, lua::Null value) {
        LUASTATE_DEBUG_LOG("  PUSH  null\n");
        lua_pushnil(luaState);
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, lua::Pointer value) {
        LUASTATE_DEBUG_LOG("  PUSH  %p\n", value);
        lua_pushlightuserdata(luaState, value);
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, Table value) {
        LUASTATE_DEBUG_LOG("  PUSH  newTable\n");
        lua_newtable(luaState);
        return 1;
    }

    template<>
    inline int push(lua_State* luaState, float value) {
        push<lua::Number>(luaState, static_cast<lua::Number>(value));
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, const std::string& value) {
        push<lua::String>(luaState, value.c_str());
        return 1;
    }

    template<typename ... Args, size_t ... Indexes>
    void push_helper(lua_State* luaState, traits::index_tuple< Indexes... >, const std::tuple<Args...>& tup)
    {
        push(luaState, std::get<Indexes>(tup)...);
    }
    
    template<typename ... Args>
    inline int push(lua_State* luaState, const std::tuple<Args...>& tuple)
    {
        push_helper(luaState, typename traits::make_indexes<Args...>::type(), tuple);
        return sizeof...(Args);
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    inline bool check(lua_State* luaState, int index);
    
    template<>
    inline bool check<lua::Integer>(lua_State* luaState, int index)
    {
        if (!lua_isnumber(luaState, index))
            return false;
        
        lua_Number eps = std::numeric_limits<lua_Number>::epsilon();
        double number = lua_tonumber(luaState, index);
        return fabs(number - static_cast<int>(number + eps)) <= eps;
    }
    
    template<>
    inline bool check<lua::Number>(lua_State* luaState, int index)
    {
        return lua_isnumber(luaState, index);
    }
    
    template<>
    inline bool check<lua::Boolean>(lua_State* luaState, int index)
    {
        return lua_isboolean(luaState, index);
    }
    
    template<>
    inline bool check<lua::String>(lua_State* luaState, int index)
    {
        // Lua is treating numbers also like strings, because they are always convertible to string
        if (lua_isnumber(luaState, index))
            return false;
        
        return lua_isstring(luaState, index);
    }
    
    template<>
    inline bool check<lua::Null>(lua_State* luaState, int index)
    {
        return lua_isnil(luaState, index);
    }
    
    template<>
    inline bool check<lua::Pointer>(lua_State* luaState, int index)
    {
        return lua_islightuserdata(luaState, index);
    }
    
    template<>
    inline bool check<lua::Table>(lua_State* luaState, int index)
    {
        return lua_istable(luaState, index);
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    inline T read(lua_State* luaState, int index);

    template<>
    inline lua::Integer read(lua_State* luaState, int index) {
        return lua_tointeger(luaState, index);
    }

    template<>
    inline lua::String read(lua_State* luaState, int index) {
        return lua_tostring(luaState, index);;
    }

    template<>
    inline lua::Number read(lua_State* luaState, int index) {
        return lua_tonumber(luaState, index);
    }

    template<>
    inline lua::Boolean read(lua_State* luaState, int index) {
        return lua_toboolean(luaState, index);
    }

    template<>
    inline lua::Null read(lua_State* luaState, int index) {
        return nullptr;
    }
    
    template<>
    inline lua::Pointer read(lua_State* luaState, int index) {
        return lua_touserdata(luaState, index);
    }

    template<>
    inline float read(lua_State* luaState, int index) {
        return read<lua::Number>(luaState, index);
    }
    
    template<>
    inline std::string read(lua_State* luaState, int index) {
        return read<lua::String>(luaState, index);
    }
    
    template<>
    inline long read(lua_State* luaState, int index) {
        return read<lua::Integer>(luaState, index);
    }
    
    template<>
    inline unsigned read(lua_State* luaState, int index) {
        return read<lua::Integer>(luaState, index);
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    inline void pop(lua_State* luaState, int n) {
        LUASTATE_DEBUG_LOG("  POP  %d\n", n);
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
    
    template<std::size_t I, typename ... Ts>
    class Pop {

        template<std::size_t... Is>
        static std::tuple<Ts...> create(lua_State* luaState, int offset, traits::indexes<Is...>) {
            return std::make_tuple(read<Ts>(luaState, Is + offset)...);
        }
        
    public:
        static std::tuple<Ts...> getTable(lua_State* luaState, int offset) {
            return create(luaState, offset, typename traits::indexes_builder<I>::index());
        }
    };
    
    template<typename ... Ts>
    inline std::tuple<Ts...> get_and_pop(lua_State* luaState) {
        constexpr size_t num = sizeof...(Ts);
        int offset = numberOfPushedValues(luaState) - num + 1;
        auto value = Pop<num, Ts...>::getTable(luaState, offset);
        stack::pop(luaState, num);
        return value;
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    inline void get(lua_State* luaState, int index) {
        LUASTATE_DEBUG_LOG("GET\n");
        lua_gettable(luaState, index);
    }
    
    template<typename T>
    inline void get(lua_State* luaState, int index, T key) {}
    
    template<>
    inline void get(lua_State* luaState, int index, const char* key) {
        LUASTATE_DEBUG_LOG("GET  %s\n", key);
        lua_getfield(luaState, index, key);
    }
    
    template<>
    inline void get(lua_State* luaState, int index, int key) {
        LUASTATE_DEBUG_LOG("GET  %d\n", key);
        lua_rawgeti(luaState, index, key);
    }
    
    inline void get_global(lua_State* luaState, const char* name) {
        LUASTATE_DEBUG_LOG("GET_GLOBAL %s\n", name);
        lua_getglobal(luaState, name);
    }
    
}}
