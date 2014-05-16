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
        
    // Check if stack leaked, we must pop 0 values
    assert(state.flushStack() == 0);
    
    return 0;
}
