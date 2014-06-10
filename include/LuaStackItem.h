//
//  
//  LuaState
//
//  Created by Simon Mikuda on 06/05/14.
//
//

#pragma once

#include <queue>

namespace lua { namespace detail {
        
    //////////////////////////////////////////////////////////////////////////////////////////////
    struct DeallocStackItem {
        int stackCap;
        int numElements;
        
        DeallocStackItem(int stackTop, int numElements) : stackCap(stackTop + numElements), numElements(numElements) {}
    };
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    struct DeallocStackComparison {
        bool operator() (const DeallocStackItem& lhs, const DeallocStackItem& rhs) const {
            return lhs.stackCap < rhs.stackCap;
        }
    };
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    typedef std::priority_queue<DeallocStackItem, std::vector<DeallocStackItem>, DeallocStackComparison> DeallocQueue;
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    struct StackItem {
        
        lua_State* state;
        detail::DeallocQueue* deallocQueue;
        
        /// Indicates number of pushed values to stack on lua::Value when created
        int top;
        
        /// Indicates number pushed values which were pushed by this lua::Value instance
        mutable int pushed;
        
        /// Indicates multi returned values, because the we want first returned value and not the last
        int grouped;
        
        StackItem() : state(nullptr), deallocQueue(nullptr)
        {
        }
        
        StackItem(lua_State* luaState, detail::DeallocQueue* deallocQueue, int stackTop, int pushedValues, int groupedValues)
        : state(luaState)
        , deallocQueue(deallocQueue)
        , top(stackTop)
        , pushed(pushedValues)
        , grouped(groupedValues)
        {
        }
        
        ~StackItem()
        {
            // Check if stack is managed automaticaly (_deallocQueue == nullptr), which is when we call C functions from Lua
            if (deallocQueue != nullptr) {
                
                // Check if we dont try to release same values twice
                int currentStackTop = stack::top(state);
                if (currentStackTop < pushed + top) {
                    return;
                }
                
                // We will check if we haven't pushed some other new lua::Value to stack
                if (top + pushed == currentStackTop) {
                    
                    // We will check deallocation priority queue, if there are some lua::Value instances to be deleted
                    while (!deallocQueue->empty() && deallocQueue->top().stackCap == top) {
                        top -= deallocQueue->top().numElements;
                        deallocQueue->pop();
                    }
                    stack::settop(state, top);
                }
                // If yes we can't pop values, we must pop it after deletion of newly created lua::Value
                // We will put this deallocation to our priority queue, so it will be deleted as soon as possible
                else
                    deallocQueue->push(detail::DeallocStackItem(top, pushed));
            }
        }
    };
} }