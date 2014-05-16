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
    // We create state and variables
    lua::State state;
    state.doString(createVariables);
    state.doString(createFunctions);
    
    // Check if stack leaked, we must pop 0 values
    assert(state.flushStack() == 0);
    
    return 0;
}
