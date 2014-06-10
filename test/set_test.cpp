//
//  state_tests.h
//  LuaState
//
//  Created by Simon Mikuda on 16/04/14.
//
//  See LICENSE and README.md files

#include "test.h"

//////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // We create state
    lua::State state;

    // Set and delete value
    state.set("number", 2.5);
    state.doString("assert(number == 2.5)");
    state.set("number", nullptr);
    state.doString("assert(number == nil)");

    // Create table with values
    state.set("tab", lua::Table());
    state["tab"].set("a", 1);
    state["tab"].set("b", 2);
    state["tab"].set("c", 3);
    state.doString("assert(tab.a == 1)");
    state.doString("assert(tab.b == 2)");
    state.doString("assert(tab.c == 3)");
    state["tab"].set("a", nullptr);
    state["tab"].set("b", nullptr);
    state["tab"].set("c", nullptr);
    state.doString("assert(tab.a == nil)");
    state.doString("assert(tab.b == nil)");
    state.doString("assert(tab.c == nil)");
    
    // Set values via lua::Value
    {
        lua::Value value = state["tab"];
        value.set(1, 1);
        value.set(2, 2);
        value.set(3, 3);
        state.doString("assert(tab[1] == 1)");
        state.doString("assert(tab[2] == 2)");
        state.doString("assert(tab[3] == 3)");
    }
    
    state.checkMemLeaks();
    return 0;
}
