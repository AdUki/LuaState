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
        
        value = state["number"];
        value = state["number"];
        value = state["number"];
        assert(function() == 10);
        assert(value == 2.5);
        assert(function() == 10);
        assert(value == 2.5);
        assert(function() == 10);
        assert(value == 2.5);
        
        value = state["getInteger"];
        function = state["number"];
        function = state["getInteger"];
        value = state["number"];
        assert(function() == 10);
        assert(value == 2.5);
        assert(function() == 10);
        assert(value == 2.5);
        assert(function() == 10);
        assert(value == 2.5);
        
        value = function;
        value = function;
        value = function;
        function = state["number"];
        assert(function == 2.5);
        assert(value() == 10);
        assert(function == 2.5);
        assert(value() == 10);
        assert(function == 2.5);
        assert(value() == 10);
    }
    
    // Test deletion in FILO order
    {
        lua::Value v1 = state["table"]["a"];
        {
            lua::Value v2 = state["table"]["b"];
            assert(v2 == 'b');
            assert(v1 == 'a');
            {
                lua::Value v3 = state["table"]["c"];
                assert(v1 == 'a');
                assert(v2 == 'b');
                assert(v3 == 'c');
            }
            assert(v2 == 'b');
            assert(v1 == 'a');
        }
        assert(v1 == 'a');
    }
    
    // Test deletion FIFO order
    lua::Value* v1 = new lua::Value(state["table"]["a"]);
    lua::Value* v2 = new lua::Value(state["table"]["b"]);
    lua::Value* v3 = new lua::Value(state["table"]["c"]);
    lua::Value* v4 = new lua::Value(state["table"]["one"]);
    lua::Value* v5 = new lua::Value(state["table"]["two"]);
    lua::Value* v6 = new lua::Value(state["table"]["three"]);
    
    assert(*v1 == 'a');
    assert(*v2 == 'b');
    assert(*v3 == 'c');
    assert(*v4 == 1);
    assert(*v5 == 2);
    assert(*v6 == 3);
    delete v1;
    
    assert(*v2 == 'b');
    assert(*v3 == 'c');
    assert(*v4 == 1);
    assert(*v5 == 2);
    assert(*v6 == 3);
    delete v2;
    
    assert(*v3 == 'c');
    assert(*v4 == 1);
    assert(*v5 == 2);
    assert(*v6 == 3);
    delete v3;
    
    assert(*v4 == 1);
    assert(*v5 == 2);
    assert(*v6 == 3);
    delete v4;
    
    assert(*v5 == 2);
    assert(*v6 == 3);
    delete v5;
    
    assert(*v6 == 3);
    delete v6;
    
    // Test deletion in random order
    v1 = new lua::Value(state["table"]["a"]);
    v2 = new lua::Value(state["table"]["b"]);
    v3 = new lua::Value(state["table"]["c"]);
    v4 = new lua::Value(state["table"]["one"]);
    v5 = new lua::Value(state["table"]["two"]);
    v6 = new lua::Value(state["table"]["three"]);
    
    assert(*v1 == 'a');
    assert(*v2 == 'b');
    assert(*v3 == 'c');
    assert(*v4 == 1);
    assert(*v5 == 2);
    assert(*v6 == 3);
    delete v6;
    
    assert(*v1 == 'a');
    assert(*v2 == 'b');
    assert(*v3 == 'c');
    assert(*v4 == 1);
    assert(*v5 == 2);
    delete v3;
    
    assert(*v1 == 'a');
    assert(*v2 == 'b');
    assert(*v4 == 1);
    assert(*v5 == 2);
    delete v1;
    
    assert(*v2 == 'b');
    assert(*v4 == 1);
    assert(*v5 == 2);
    delete v5;
    
    assert(*v2 == 'b');
    assert(*v4 == 1);
    delete v4;
    
    assert(*v2 == 'b');
    delete v2;
    
    // Test copying values
    v1 = new lua::Value(state["table"]["a"]);
    v2 = new lua::Value(*v1);
    v3 = new lua::Value(*v2);
    
    assert(*v1 == 'a');
    assert(*v2 == 'a');
    assert(*v3 == 'a');
    delete v1;
    
    assert(*v2 == 'a');
    assert(*v3 == 'a');
    delete v2;
    
    assert(*v3 == 'a');
    delete v3;
    
    // Test moving and copying values to functions
    {
        auto constCopyValueFnc = [](const lua::Value&  value){ assert(value == 3); };
        auto copyValueFnc =      [](const lua::Value&  value){ assert(value == 3); };
        auto moveValueFnc =      [](      lua::Value&& value){ assert(value == 3); };
        
        constCopyValueFnc(state["table"]["three"]);
        copyValueFnc(state["table"]["three"]);
        moveValueFnc(state["table"]["three"]);
        
        lua::Value value = state["table"]["three"];
        constCopyValueFnc(value);
        copyValueFnc(value);
        moveValueFnc(state["table"]["three"]);
    }
    
    // Test geting value from Lua and sending value back to Lua
    state.doString("function moveValues(tab)  assert(tab.a == 'a')  assert(tab.b == 'b')  assert(tab.c == 'c')  assert(tab.one == 1)  assert(tab.two == 2)  assert(tab.three == 3) end");
    
    state.doString("moveValues(table)");
    
    state["moveValues"](state["table"]);
    state["moveValues"](state["table"], state["table"]);
    
    v1 = new lua::Value(state["table"]);
    state["moveValues"](*v1);
    state["moveValues"](*v1);
    delete v1;
    
    state.checkMemLeaks();
    return 0;
}
