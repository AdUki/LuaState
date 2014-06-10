//
//  state_tests.h
//  LuaState
//
//  Created by Simon Mikuda on 16/04/14.
//
//  See LICENSE and README.md files

#include "test.h"

#include <fstream>

//////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    lua::State state;

    state.doString("number = 10");
    state.doString("assert(number == 10)");
    
    try {
        state.doString("we will invoke syntax error");
        assert(false);
    } catch (lua::LoadError ex) {
        printf("%s\n", ex.what());
    }
    
    try {
        state.doString("nofunction()");
        assert(false);
    } catch (lua::RuntimeError ex) {
        printf("%s\n", ex.what());
    }
    
    std::ofstream luaFile;

    try {
        state.doFile("no_file_here");
        assert(false);
    } catch (lua::LoadError ex) {
        printf("%s ", ex.what());
    }

    luaFile.open("test.lua");
    luaFile << "local number = 100; assert(number == 100)" << std::endl;
    luaFile.close();
    state.doFile("test.lua");
    
    luaFile.open("test.lua");
    luaFile << "nofunction()" << std::endl;
    luaFile.close();
    try {
        state.doFile("test.lua");
        assert(false);
    } catch (lua::RuntimeError ex) {
        printf("%s\n", ex.what());
    }
    
    // Check returning values from doString
    int a1, a2, a3;
    a1 = state.doString("return 10");
    assert(a1 == 10);
    lua::tie(a1) = state.doString("return 100");
    assert(a1 == 100);
    lua::tie(a1, a2, a3) = state.doString("return 11, 12, 13");
    assert(a1 == 11 && a2 == 12 && a3 == 13);
    lua::tie(a1) = state.doString("return 21, 12, 13");
    assert(a1 == 21);
    lua::tie(a1, a2, a3) = state.doString("return 11");
    assert(a1 == 11);
    
    // Check returning values from doFile
    luaFile.open("test.lua");
    luaFile << "return 11, 12, 13" << std::endl;
    luaFile.close();
    
    a1 = state.doFile("test.lua");
    assert(a1 == 11);
    lua::tie(a1, a2, a3) = state.doFile("test.lua");
    assert(a1 == 11 && a2 == 12 && a3 == 13);
    
    state.checkMemLeaks();
    return 0;
}
