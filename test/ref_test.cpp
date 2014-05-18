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

    lua::Ref ref = state["table"]["a"];
    lua::Ref tabRef = state["table"];
    
    assert(ref.unref() == 'a');
    assert(tabRef.unref()["a"] == 'a');
    
    lua::Ref copyRef = ref;
    assert(copyRef.unref() == 'a');
    
    copyRef = tabRef;
    assert(copyRef.unref()["a"] == 'a');
        
    // Check if stack leaked, we must pop 0 values
    assert(state.flushStack() == 0);
    
    return 0;
}
