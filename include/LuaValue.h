//
//  LuaValue.h
//  LuaState
//
//  Created by Simon Mikuda on 17/03/14.
//
//

#pragma once

#include <tuple>
#include <cstring>
#include "./LuaStack.h"
#include "./LuaPrimitives.h"
#include "./LuaFunction.h"
#include "./LuaKey.h"

static void stackDump (lua_State *L)
{
    int i;
    int top = lua_gettop(L);
    printf("========= %d \n", top);
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
    printf("=========\n");
}

namespace lua {

    //////////////////////////////////////////////////////////////////////////////////////////////////
    class Value
    {
        std::shared_ptr<lua_State> _luaState;
        
        mutable int _pushedValues;
        const int _stackTop;
        Key _key;
        
        //////////////////////////////////////////////////////////////////////////////////////////////////
        void checkStack() const {
            if (stack::numberOfPushedValues(_luaState.get()) - _stackTop != _pushedValues) {
                throw StackError(stack::numberOfPushedValues(_luaState.get()), _pushedValues);
            }
        }
        
    public:
        // constructors
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        Value(const std::shared_ptr<lua_State>& luaState, const char* name)
        : _luaState(luaState)
        , _pushedValues(2)
        , _stackTop(stack::numberOfPushedValues(_luaState.get()))
        , _key(name) {
            stack::push(luaState.get(), name);
            stack::get(_luaState.get(), LUA_GLOBALSINDEX, name);
        }
        
        ~Value() {
            if (_luaState != nullptr)
                stack::pop(_luaState.get(), _pushedValues);
        }
        
        Value(const Value&) = delete;
        Value(Value&&) = default;
        
        // operator overloads
        //////////////////////////////////////////////////////////////////////////////////////////////////
        
        Value& operator= (const Value &) = delete;
        Value& operator= (Value &&) = default;
        
        template<typename T>
        Value&& operator[](T key) {
            checkStack();
            
            stack::push(_luaState.get(), key);
            stack::get(_luaState.get(), -2, key);
            _pushedValues += 2;
            
            return std::move(*this);
        }

        Function operator()() const {
            checkStack();
            _pushedValues -= 1;
            return Function(_luaState, 0);
        }
        
        template<typename... Ts>
        Function operator()(Ts... args) const {
            checkStack();
            _pushedValues -= 1;
            stack::push(_luaState.get(), args...);
            return Function(_luaState, sizeof...(args));
        }
        
        template<typename T>
        operator T() const {
            checkStack();
            stackDump(_luaState.get());
            auto retValue = stack::read<T>(_luaState.get(), -1);
            stack::pop(_luaState.get(), 2);
            _pushedValues -= 2;
            stackDump(_luaState.get());
            return retValue;
        }
        
        template<typename T>
        void operator= (const T& value) const {
            checkStack();
            
            stack::pop(_luaState.get(), 1);
            
            if (_pushedValues == 2) {
                LuaType::String name = stack::pop_front<LuaType::String>(_luaState.get());
                stack::push(_luaState.get(), value);
                lua_setglobal(_luaState.get(), name);
            }
            else {
                stack::push(_luaState.get(), value);
                lua_settable(_luaState.get(), -3);
            }
            _pushedValues -= 2;
        }

        // other functions
        //////////////////////////////////////////////////////////////////////////////////////////////////

        // TODO: getTable, ktora ma variabilny pocet parametrov ako kluce
//        template <typename... Ret>
//        std::tuple<Ret...> getTable() const {
//            auto returnValue = stack::get<Ret...>(_luaState);
//            lua_settop(_luaState, 0);
//            
//            return returnValue;
//        }
        
    };
    
    // compare operators
    //////////////////////////////////////////////////////////////////////////////////////////////////
    inline bool operator==(const Value &value, const char *string)        { return strcmp(value, string) == 0; }
    inline bool operator==(const char *string, const Value &value)        { return strcmp(value, string) == 0; }
    inline bool operator==(const Value &value, const std::string& string) { return strcmp(value, string.c_str()) == 0; }
    inline bool operator==(const std::string& string, const Value &value) { return strcmp(value, string.c_str()) == 0; }
    
    template <typename T>
    inline bool operator==(const Value &stateValue, const T& value) {
        return T(stateValue) == value;
    }
    template <typename T>
    inline bool operator==(const T& value, const Value &stateValue) {
        return T(stateValue) == value;
    }
    
}

