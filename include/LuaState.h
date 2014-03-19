#include <cassert>
#include <string>
#include <functional>
#include <memory>

#include <lua.hpp>

#include "./LuaException.h"
#include "./LuaValue.h"
#include "./LuaTable.h"

namespace lua {
    
    class State
    {
        std::shared_ptr<lua_State> _luaState;
        
    public:
        // constructors
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        State() {
            lua_State* luaState = luaL_newstate();
            luaL_openlibs(luaState);
            assert(luaState != NULL);
            
            _luaState = std::shared_ptr<lua_State>(luaState, std::bind(&lua_close, luaState));
        }
        
        ~State() {
        }
        
        State(const State&) = delete;
        State(State&&) = default;
        
        // operators overloads
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        State& operator= (const State &) = delete;
        State& operator= (State &&) = default;
        
        Value operator[](LuaType::String name) {
            return Value(_luaState, name);
        }
        
        // Allow automatic casting when used in comparisons
        bool operator==(Value &other) = delete;
        
        // other functions
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        void doFile(const std::string& filePath) {
            if (luaL_dofile(_luaState.get(), filePath.c_str())) {
                std::string message = stack::read<std::string>(_luaState.get(), -1);
                stack::pop(_luaState.get(), 1);
                throw LoadError(message);
            }
        }
        void doString(const std::string& string) {
            if (luaL_dostring(_luaState.get(), string.c_str())) {
                std::string message = stack::read<std::string>(_luaState.get(), -1);
                stack::pop(_luaState.get(), 1);
                throw LoadError(message);
            }
        }
        
        /// @returns Number of flushed items
        int flushStack() {
            int count = stack::numberOfPushedValues(_luaState.get());
            lua_settop(_luaState.get(), 0);
            return count;
        }
        
    };
}
