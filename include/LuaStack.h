//
//  LuaStack.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files

#pragma once

namespace lua { namespace stack {
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    inline int top(const std::shared_ptr<lua_State>& luaState) {
        return lua_gettop(luaState.get());
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////

    inline int push(const std::shared_ptr<lua_State>& luaState) { return 0; }
    
    template<typename T>
    inline int push(const std::shared_ptr<lua_State>& luaState, T value);
    
    template<typename T, typename ... Ts>
    inline int push(const std::shared_ptr<lua_State>& luaState, const T value, const Ts... values) {
        push(luaState, value);
        push(luaState, values...);
        return sizeof...(Ts) + 1;
    }

    template<>
    inline int push(const std::shared_ptr<lua_State>& luaState, int value) {
        LUASTATE_DEBUG_LOG("  PUSH  %d\n", value);
        lua_pushinteger(luaState.get(), value);
        return 1;
    }

    template<>
    inline int push(const std::shared_ptr<lua_State>& luaState, lua::String value) {
        LUASTATE_DEBUG_LOG("  PUSH  %s\n", value);
        lua_pushstring(luaState.get(), value);
        return 1;
    }

    template<>
    inline int push(const std::shared_ptr<lua_State>& luaState, lua::Number value) {
        LUASTATE_DEBUG_LOG("  PUSH  %lf\n", value);
        lua_pushnumber(luaState.get(), value);
        return 1;
    }

    template<>
    inline int push(const std::shared_ptr<lua_State>& luaState, lua::Boolean value) {
        LUASTATE_DEBUG_LOG("  PUSH  %s\n", value ? "true" : "false");
        lua_pushboolean(luaState.get(), value);
        return 1;
    }

    template<>
    inline int push(const std::shared_ptr<lua_State>& luaState, lua::Null value) {
        LUASTATE_DEBUG_LOG("  PUSH  null\n");
        lua_pushnil(luaState.get());
        return 1;
    }
    
    template<>
    inline int push(const std::shared_ptr<lua_State>& luaState, lua::Pointer value) {
        LUASTATE_DEBUG_LOG("  PUSH  %p\n", value);
        lua_pushlightuserdata(luaState.get(), value);
        return 1;
    }
    
    template<>
    inline int push(const std::shared_ptr<lua_State>& luaState, Table value) {
        LUASTATE_DEBUG_LOG("  PUSH  newTable\n");
        lua_newtable(luaState.get());
        return 1;
    }

    template<>
    inline int push(const std::shared_ptr<lua_State>& luaState, float value) {
        push<lua::Number>(luaState, static_cast<lua::Number>(value));
        return 1;
    }
    
    template<>
    inline int push(const std::shared_ptr<lua_State>& luaState, const std::string& value) {
        push<lua::String>(luaState, value.c_str());
        return 1;
    }

    template<typename ... Args, size_t ... Indexes>
    void push_helper(const std::shared_ptr<lua_State>& luaState, traits::index_tuple< Indexes... >, const std::tuple<Args...>& tup)
    {
        push(luaState, std::get<Indexes>(tup)...);
    }
    
    template<typename ... Args>
    inline int push(const std::shared_ptr<lua_State>& luaState, const std::tuple<Args...>& tuple)
    {
        push_helper(luaState, typename traits::make_indexes<Args...>::type(), tuple);
        return sizeof...(Args);
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    inline bool check(const std::shared_ptr<lua_State>& luaState, int index);
    
    template<>
    inline bool check<lua::Integer>(const std::shared_ptr<lua_State>& luaState, int index)
    {
        if (!lua_isnumber(luaState.get(), index))
            return false;
        
        lua_Number eps = std::numeric_limits<lua_Number>::epsilon();
        double number = lua_tonumber(luaState.get(), index);
        return fabs(number - static_cast<int>(number + eps)) <= eps;
    }
    
    template<>
    inline bool check<lua::Number>(const std::shared_ptr<lua_State>& luaState, int index)
    {
        return lua_isnumber(luaState.get(), index);
    }
    
    template<>
    inline bool check<lua::Boolean>(const std::shared_ptr<lua_State>& luaState, int index)
    {
        return lua_isboolean(luaState.get(), index);
    }
    
    template<>
    inline bool check<lua::String>(const std::shared_ptr<lua_State>& luaState, int index)
    {
        // Lua is treating numbers also like strings, because they are always convertible to string
        if (lua_isnumber(luaState.get(), index))
            return false;
        
        return lua_isstring(luaState.get(), index);
    }
    
    template<>
    inline bool check<lua::Null>(const std::shared_ptr<lua_State>& luaState, int index)
    {
        return lua_isnil(luaState.get(), index);
    }
    
    template<>
    inline bool check<lua::Pointer>(const std::shared_ptr<lua_State>& luaState, int index)
    {
        return lua_islightuserdata(luaState.get(), index);
    }
    
    template<>
    inline bool check<lua::Table>(const std::shared_ptr<lua_State>& luaState, int index)
    {
        return lua_istable(luaState.get(), index);
    }

    template<>
    inline bool check<lua::Callable>(const std::shared_ptr<lua_State>& luaState, int index)
    {
        lua_State* state = luaState.get();
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
    
    //////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    inline T read(const std::shared_ptr<lua_State>& luaState, int index);

    template<>
    inline lua::Integer read(const std::shared_ptr<lua_State>& luaState, int index) {
        return lua_tointeger(luaState.get(), index);
    }

    template<>
    inline lua::String read(const std::shared_ptr<lua_State>& luaState, int index) {
        return lua_tostring(luaState.get(), index);;
    }

    template<>
    inline lua::Number read(const std::shared_ptr<lua_State>& luaState, int index) {
        return lua_tonumber(luaState.get(), index);
    }

    template<>
    inline lua::Boolean read(const std::shared_ptr<lua_State>& luaState, int index) {
        return lua_toboolean(luaState.get(), index);
    }

    template<>
    inline lua::Null read(const std::shared_ptr<lua_State>& luaState, int index) {
        return nullptr;
    }
    
    template<>
    inline lua::Pointer read(const std::shared_ptr<lua_State>& luaState, int index) {
        return lua_touserdata(luaState.get(), index);
    }

    template<>
    inline float read(const std::shared_ptr<lua_State>& luaState, int index) {
        return read<lua::Number>(luaState, index);
    }
    
    template<>
    inline std::string read(const std::shared_ptr<lua_State>& luaState, int index) {
        return read<lua::String>(luaState, index);
    }
    
    template<>
    inline long read(const std::shared_ptr<lua_State>& luaState, int index) {
        return read<lua::Integer>(luaState, index);
    }
    
    template<>
    inline unsigned read(const std::shared_ptr<lua_State>& luaState, int index) {
        return read<lua::Integer>(luaState, index);
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    inline void settop(const std::shared_ptr<lua_State>& luaState, int n) {
        LUASTATE_DEBUG_LOG("  POP  %d\n", top(luaState) - n);
        lua_settop(luaState.get(), n);
    }
    
    inline void pop(const std::shared_ptr<lua_State>& luaState, int n) {
        LUASTATE_DEBUG_LOG("  POP  %d\n", n);
        lua_pop(luaState.get(), n);
    }
    
    template<typename T>
    inline T pop_front(const std::shared_ptr<lua_State>& luaState) {
        T value = read<T>(luaState, 1);
        lua_remove(luaState.get(), 0);
        return value;
    }
    
    template<typename T>
    inline T pop_back(const std::shared_ptr<lua_State>& luaState) {
        T value = read<T>(luaState.get(), -1);
        pop(luaState, 1);
        return value;
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    inline void get(const std::shared_ptr<lua_State>& luaState, int index) {
        LUASTATE_DEBUG_LOG("GET\n");
        lua_gettable(luaState.get(), index);
    }
    
    template<typename T>
    inline void get(const std::shared_ptr<lua_State>& luaState, int index, T key) {}
    
    template<>
    inline void get(const std::shared_ptr<lua_State>& luaState, int index, const char* key) {
        LUASTATE_DEBUG_LOG("GET  %s\n", key);
        lua_getfield(luaState.get(), index, key);
    }
    
    template<>
    inline void get(const std::shared_ptr<lua_State>& luaState, int index, int key) {
        LUASTATE_DEBUG_LOG("GET  %d\n", key);
        lua_rawgeti(luaState.get(), index, key);
    }
    
    inline void get_global(const std::shared_ptr<lua_State>& luaState, const char* name) {
        LUASTATE_DEBUG_LOG("GET_GLOBAL %s\n", name);
        lua_getglobal(luaState.get(), name);
    }
    
}}
