//
//  LuaStack.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files

#pragma once

#include <limits>

namespace lua { namespace stack {
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    
    inline int top(lua_State* luaState) {
        return lua_gettop(luaState);
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    
    template<typename T>
    inline int push(lua_State* luaState, T value);
    
    template<typename T, typename ... Ts>
    inline int push(lua_State* luaState, T value, Ts... values) {
        push(luaState, std::forward<T>(value));
        push(luaState, values...);
        return sizeof...(Ts) + 1;
    }
    
    template<typename ... Args, size_t ... Indexes>
    void push_tuple(lua_State* luaState, traits::index_tuple< Indexes... >, const std::tuple<Args...>& tup)
    {
        push(luaState, std::get<Indexes>(tup)...);
    }
    
    template<typename ... Args>
    inline int push(lua_State* luaState, const std::tuple<Args...>& tuple)
    {
        push_tuple(luaState, typename traits::make_indexes<Args...>::type(), tuple);
        return sizeof...(Args);
    }
    
    inline int push(lua_State* luaState) { return 0; }

    template<>
    inline int push(lua_State* luaState, int value) {
        LUASTATE_DEBUG_LOG("  PUSH  %d", value);
        lua_pushnumber(luaState, value);
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, short int value) {
        LUASTATE_DEBUG_LOG("  PUSH  %d", value);
        lua_pushnumber(luaState, value);
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, long long int value) {
        LUASTATE_DEBUG_LOG("  PUSH  %lld", value);
        lua_pushnumber(luaState, value);
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, unsigned short int value) {
        LUASTATE_DEBUG_LOG("  PUSH  %d", value);
        lua_pushnumber(luaState, value);
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, unsigned int value) {
        LUASTATE_DEBUG_LOG("  PUSH  %ud", value);
        lua_pushnumber(luaState, value);
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, long int value) {
        LUASTATE_DEBUG_LOG("  PUSH  %ld", value);
        lua_pushnumber(luaState, value);
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, unsigned long int value) {
        LUASTATE_DEBUG_LOG("  PUSH  %lud", value);
        lua_pushnumber(luaState, value);
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, unsigned long long int value) {
        LUASTATE_DEBUG_LOG("  PUSH  %llud", value);
        lua_pushnumber(luaState, value);
        return 1;
    }

    template<>
    inline int push(lua_State* luaState, char value) {
        LUASTATE_DEBUG_LOG("  PUSH  %c\n", value);
        char string[2];
        string[0] = value;
        string[1] = '\0';
        lua_pushstring(luaState, string);
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, unsigned char value) {
        LUASTATE_DEBUG_LOG("  PUSH  %c\n", value);
        char string[2];
        string[0] = static_cast<char>(value);
        string[1] = '\0';
        lua_pushstring(luaState, string);
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, const char* value) {
        LUASTATE_DEBUG_LOG("  PUSH  %s", value);
        lua_pushstring(luaState, value);
        return 1;
    }

    template<>
    inline int push(lua_State* luaState, std::string value) {
        LUASTATE_DEBUG_LOG("  PUSH  %s", value.c_str());
        lua_pushstring(luaState, value.c_str());
        return 1;
    }

    template<>
    inline int push(lua_State* luaState, const unsigned char* value) {
        LUASTATE_DEBUG_LOG("  PUSH  %s", value);
        lua_pushstring(luaState, reinterpret_cast<const char*>(value));
        return 1;
    }

    template<>
    inline int push(lua_State* luaState, float value) {
        LUASTATE_DEBUG_LOG("  PUSH  %lf\n", value);
        lua_pushnumber(luaState, value);
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, double value) {
        LUASTATE_DEBUG_LOG("  PUSH  %lf\n", value);
        lua_pushnumber(luaState, value);
        return 1;
    }
    
    template<>
    inline int push(lua_State* luaState, long double value) {
        LUASTATE_DEBUG_LOG("  PUSH  %Lf\n", value);
        lua_pushnumber(luaState, value);
        return 1;
    }

    template<>
    inline int push(lua_State* luaState, lua::Boolean value) {
        LUASTATE_DEBUG_LOG("  PUSH  %s", value ? "true" : "false");
        lua_pushboolean(luaState, value);
        return 1;
    }

    template<>
    inline int push(lua_State* luaState, lua::Nil value) {
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
    
    inline int push_str(lua_State* luaState, const char* value, int lenght) {
        LUASTATE_DEBUG_LOG("  PUSH  %s", value);
        lua_pushlstring(luaState, value, lenght);
        return 1;
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////

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
    inline bool check<lua::Nil>(lua_State* luaState, int index)
    {
        return lua_isnoneornil(luaState, index);
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
    
    template<>
    inline bool check<unsigned>(lua_State* luaState, int index)
    {
        return lua_isnumber(luaState, index);
    }
    
    template<>
    inline bool check<float>(lua_State* luaState, int index)
    {
        return lua_isnumber(luaState, index);
    }

    template<>
    inline bool check<lua::Callable>(lua_State* luaState, int index)
    {
        lua_State* state = luaState;
        bool isCallable = lua_isfunction(state, index) || lua_iscfunction(state, index);
        
        if (!isCallable) {
            lua_getmetatable(state, index);
            if (lua_istable(state, -1)) {
                lua_pushstring(state, "__call");
                lua_rawget(state, -2);
                isCallable = !lua_isnil(state, -1);
                lua_pop(state, 1);
            }
            lua_pop(state, 1);
        }
        
        return isCallable;
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    inline T read(lua_State* luaState, int index);

    template<>
    inline int read(lua_State* luaState, int index) {
        return lua_tointeger(luaState, index);
    }
    
    template<>
    inline long read(lua_State* luaState, int index) {
        return static_cast<long>(lua_tointeger(luaState, index));
    }
    
    template<>
    inline long long read(lua_State* luaState, int index) {
        return static_cast<long long>(lua_tointeger(luaState, index));
    }
    
    template<>
    inline short read(lua_State* luaState, int index) {
        return static_cast<short>(lua_tointeger(luaState, index));
    }
    
    template<>
    inline unsigned read(lua_State* luaState, int index) {
#if LUA_VERSION_NUM > 501
        return static_cast<unsigned>(lua_tounsigned(luaState, index));
#else
        return static_cast<unsigned>(lua_tointeger(luaState, index));
#endif
    }
    
    template<>
    inline unsigned short read(lua_State* luaState, int index) {
#if LUA_VERSION_NUM > 501
        return static_cast<unsigned short>(lua_tounsigned(luaState, index));
#else
        return static_cast<unsigned short>(lua_tointeger(luaState, index));
#endif
    }
    
    template<>
    inline unsigned long read(lua_State* luaState, int index) {
#if LUA_VERSION_NUM > 501
        return static_cast<unsigned long>(lua_tounsigned(luaState, index));
#else
        return static_cast<unsigned long>(lua_tointeger(luaState, index));
#endif
    }
    
    template<>
    inline unsigned long long read(lua_State* luaState, int index) {
#if LUA_VERSION_NUM > 501
        return static_cast<unsigned long long>(lua_tounsigned(luaState, index));
#else
        return static_cast<unsigned long long>(lua_tointeger(luaState, index));
#endif
    }

    template<>
    inline double read(lua_State* luaState, int index) {
        return static_cast<double>(lua_tonumber(luaState, index));
    }
    
    template<>
    inline long double read(lua_State* luaState, int index) {
        return static_cast<long double>(lua_tonumber(luaState, index));
    }
    
    template<>
    inline float read(lua_State* luaState, int index) {
        return static_cast<float>(lua_tonumber(luaState, index));
    }

    template<>
    inline bool read(lua_State* luaState, int index) {
        return lua_toboolean(luaState, index);
    }

    template<>
    inline lua::Nil read(lua_State* luaState, int index) {
        return nullptr;
    }
    
    template<>
    inline lua::Pointer read(lua_State* luaState, int index) {
        return lua_touserdata(luaState, index);
    }
    
    template<>
    inline char read(lua_State* luaState, int index) {
        return static_cast<char>(lua_tostring(luaState, index)[0]);
    }
    
    template<>
    inline unsigned char read(lua_State* luaState, int index) {
        return static_cast<unsigned char>(lua_tostring(luaState, index)[0]);
    }
    
    template<>
    inline const char* read(lua_State* luaState, int index) {
        return lua_tostring(luaState, index);
    }
    
    template<>
    inline std::string read(lua_State* luaState, int index) {
        return lua_tostring(luaState, index);
    }

    template<>
    inline const unsigned char* read(lua_State* luaState, int index) {
        return reinterpret_cast<const unsigned char*>(lua_tostring(luaState, index));;
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    
    inline void settop(lua_State* luaState, int n) {
        LUASTATE_DEBUG_LOG("  POP  %d", top(luaState) - n);
        lua_settop(luaState, n);
    }
    
    inline void pop(lua_State* luaState, int n) {
        LUASTATE_DEBUG_LOG("  POP  %d", n);
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
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    
    inline void get(lua_State* luaState, int index) {
        LUASTATE_DEBUG_LOG("GET table %d", index);
        lua_gettable(luaState, index);
    }
    
    template<typename T>
    inline void get(lua_State* luaState, int index, T key) {}
    
    template<>
    inline void get(lua_State* luaState, int index, const char* key) {
        LUASTATE_DEBUG_LOG("GET  %s", key);
        lua_getfield(luaState, index, key);
    }
    
    template<>
    inline void get(lua_State* luaState, int index, int key) {
        LUASTATE_DEBUG_LOG("GET  %d", key);
        lua_rawgeti(luaState, index, key);
    }
    
    inline void get_global(lua_State* luaState, const char* name) {
        LUASTATE_DEBUG_LOG("GET_GLOBAL %s", name);
        lua_getglobal(luaState, name);
    }
    
}}
