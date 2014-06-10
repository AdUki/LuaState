//
//  get_tests.h
//  LuaState
//
//  Created by Simon Mikuda on 16/04/14.
//
//  See LICENSE and README.md files


#include "test.h"

//////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // We create state and variables
    lua::State state;
    state.doString(createVariables);
    state.doString(createFunctions);
    
    // Test indexes
    assert(state["table"][1] == 100);
    assert(strcmp(state["table"][2], "hello") == 0);
    assert(state["table"][3] == true);
    
    // Test fields
    assert(state["table"]["one"] == 1);
    assert(state["table"]["two"] == 2);
    assert(state["table"]["three"] == 3);
    
    assert(state["table"]["a"] == 'a');
    assert(state["table"]["b"] == 'b');
    assert(state["table"]["c"] == 'c');
    
    // Test nesting tables
    assert(state["nested"]["table"]["one"] == 1);
    assert(state["nested"]["nested"]["table"]["two"] == 2);
    assert(state["nested"]["nested"]["nested"]["table"]["three"] == 3);
    
    assert(state["nested"]["nested"]["nested"]["nested"]["table"]["a"] == 'a');
    assert(state["nested"]["nested"]["table"]["b"] == 'b');
    assert(state["nested"]["nested"]["nested"]["nested"]["nested"]["nested"]["table"]["c"] == 'c');
    
    // Test function return values
    assert(state["getInteger"]() == 10);
    assert(state["getValues"]() == 1);
    
    // Test function multi return values
    int a, b, c, d;
    lua::tie(a) = state["getValues"]();
    assert(a == 1);
    lua::tie(a, b) = state["getValues"]();
    assert(a == 1 && b == 2);
    lua::tie(a, b, c) = state["getValues"]();
    assert(a == 1 && b == 2 && c == 3);
    lua::tie(a, b, c, d) = state["getValues"]();
    assert(a == 1 && b == 2 && c == 3);
    
    // Test mixed nesting
    assert(state["getTable"]()[1] == 100);
    assert(state["getTable"]()["a"] == 'a');
    
    assert(state["getNested"]()["func"]()["func"]()["func"]()["table"][1] == 100);
    assert(state["getNested"]()["func"]()["func"]()["func"]()["table"]["a"] == 'a');
    
    // Test mixed nesting with multi return
    assert(state["getNestedValues"]() == 1);
    
    {
        lua::Value test;
        lua::tie(a, test, c) = state["getNestedValues"]();
        assert(a == 1 && c == 3);
        assert(test[1] == 1);
        assert(test[2] == 2);
        assert(test[3] == 3);
    }
    
    // Test get function
    lua::Integer integerValue;
    lua::Number numberValue;
    lua::String stringValue;
    lua::Boolean boolValue = false;
    
    if (state["integer"].get(stringValue))
        assert(false);
    if (state["integer"].get(integerValue))
        assert(integerValue == 10);
    else
        assert(false);
    
    if (state["text"].get(integerValue))
        assert(false);
    if (state["text"].get(stringValue))
        assert(strcmp(stringValue, "hello") == 0);
    else
        assert(false);
    
    if (state["boolean"].get(stringValue))
        assert(false);
    if (state["boolean"].get(boolValue))
        assert(boolValue == true);
    else
        assert(false);
    
    if (state["number"].get(stringValue))
        assert(false);
    if (state["number"].get(integerValue))
        assert(false);
    if (state["number"].get(numberValue))
        assert(numberValue == 2.5);
    else
        assert(false);
    
    state.checkMemLeaks();
    
    return 0;
}
