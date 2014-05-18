//
//  state_tests.h
//  LuaState
//
//  Created by Simon Mikuda on 16/04/14.
//
//  See LICENSE and README.md files

#include "test.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    lua::State state;
    state.doString(createVariables);
    state.doString(createFunctions);
    
    {
        // Test move sematics with lvalue
        assert(state["number"] == 2.5);
        assert(state["getValues"]() == 1);
        
        // Test copy sematics with rvalue
        lua::Value function = state["getInteger"];
        lua::Value value = state["number"];
        assert(function() == 10);
        assert(value == 2.5);
        assert(function() == 10);
        assert(value == 2.5);
        assert(function() == 10);
        assert(value == 2.5);
    }
    
    // Test deletion in FIFO order
    // TODO: add tests
    
    // Test deletion in random order
    // TODO: add tests
    
    // Test copying values
    // TODO: add tests
    
    // Test moving values
    // TODO: add tests
    
    // Check if stack leaked, we must pop 0 values
    assert(state.flushStack() == 0);
    
    return 0;
}
