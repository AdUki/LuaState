//
//  LuaReturn.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files

#pragma once

namespace lua {
    
    namespace stack {
        
        inline lua::Value readValue(const std::shared_ptr<lua_State>& luaState, detail::DeallocQueue* deallocQueue, int index) {
            return lua::Value(luaState, deallocQueue, index);
        }
        
        template<std::size_t I, typename ... Ts>
        class Pop {
            
            template<std::size_t... Is>
            static std::tuple<Ts...> create(const std::shared_ptr<lua_State>& luaState, detail::DeallocQueue* deallocQueue, int offset, traits::indexes<Is...>) {
                return std::make_tuple(readValue(luaState, deallocQueue, Is + offset)...);
            }
            
        public:
            static std::tuple<Ts...> getTable(const std::shared_ptr<lua_State>& luaState, detail::DeallocQueue* deallocQueue, int offset) {
                return create(luaState, deallocQueue, offset, typename traits::indexes_builder<I>::index());
            }
        };
        
        template<typename ... Ts>
        inline std::tuple<Ts...> get_and_pop(const std::shared_ptr<lua_State>& luaState, detail::DeallocQueue* deallocQueue) {
            constexpr size_t num = sizeof...(Ts);
            int offset = top(luaState) - num + 1;
            auto value = Pop<num, Ts...>::getTable(luaState, deallocQueue, offset);
            return value;
        }
        
        
    }
    
    /// Class for automaticly cas lua::Function instance to multiple return values with lua::tie
	template <typename ... Ts>
	class Return
    {
        /// Return values
	    std::tuple<Ts&...> _tuple;
        
	public:
        
        /// Constructs class with given arguments
        ///
        /// @param args    Return values
	    Return(Ts&... args)
        : _tuple(args...) {}
        
        /// Operator sets values to std::tuple
        ///
        /// @param function     Function being called
	    void operator= (const Value& value) {
            
            // We will take pushed values and add them to returned lua::Values
            value._pushedValues = 0;
            
            _tuple = stack::get_and_pop<typename std::remove_reference<Ts>::type...>(value._luaState, value._deallocQueue);
	    }

	};
    
    /// Use this function when you want to retrieve multiple return values from lua::Function
    template <typename ... Ts>
    Return<Ts&...> tie(Ts&... args) {
        return Return<Ts&...>(args...);
    }
}
