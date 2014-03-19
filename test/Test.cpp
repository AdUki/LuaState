#include "../include/LuaState.h"

#define check(code, result) \
{ \
    if (code == result) { \
        printf("OK %s == %s\n", #code, #result); \
    } \
    else { \
        printf("FAIL %s\n", #code); \
        assert(false); \
    } \
}

int main(int argc, char** argv)
{
    // State creation doString and loadFile
    lua::State state;
    state.doString("a = 5");
    printf("OK\n");
    
    try {
        state.doString("a()");
        assert(false);
    } catch (lua::LoadError ex) {
        printf("%s ", ex.what());
        printf("OK\n");
    }
    
    try {
        state.doFile("bum");
        assert(false);
    } catch (lua::LoadError ex) {
        printf("%s ", ex.what());
        printf("OK\n");
    }
    
    check(state["a"], 5);
    
    
    state.doString("a = 'ahoj'");
    std::string text = state["a"];
    check(text, "ahoj");
    check(state["a"], "ahoj");
    check(state["a"], text);
    const char* cText = state["a"];
    check(strcmp(cText, "ahoj"), 0);


    state.doString("a = true");
    bool boolValue = state["a"];
    check(boolValue, true);


    state.doString("tab = { a = 1; b = '2'; ct = { 10, 20, 30 } }");
    check(state["tab"]["a"], 1);
    check(state["tab"]["b"], "2");
    check(state["tab"]["ct"][1], 10);
    check(state["tab"]["ct"][2], 20);
    check(state["tab"]["ct"][3], 30);
    
    {
        lua::Value table = state["tab"]["ct"];
        int ct1 = table[1];
        int ct2 = table[2];
        int ct3 = table[3];
        check(ct1, 10);
        check(ct2, 20);
        check(ct3, 30);
        
        {
            lua::Value table2 = state["tab"];
            int tab = table2["a"];
            check(tab, 1);
            
            try {
                ct2 = table[1];
                assert(false);
            } catch (lua::StackError exp) {
                check(ct2, 20);
            }
        }
        ct2 = table[1];
        check(ct2, 10);
    }
    
    int intValue = state["tab"]["ct"][2];
    check(intValue, 20);
    
    state.doString("function setGlobalTest(name, value) testGlobal = 666 end");
    state["setGlobalTest"]();
    check(state["testGlobal"], 666);
    
    state.doString("function setGlobal(name, value) _G[name] = value end");
    state["setGlobal"]("a", 1000000);
    check(state["a"], 1000000);
    
    state.doString("function add(a,b) return a+b end");
    intValue = state["add"](5,7);
    check(intValue, 12);

    state.doString("function logicOr(a,b) return a or b end");
    boolValue = state["logicOr"](true,false);
    check(boolValue, true);

    state.doString("function cat(a,b) return a .. b end");
    cText = state["cat"]("aa","bb");
//    text = state["cat"]("aa","bb");
    text = cText; // TODO: fix na string
    check(text, "aabb");
    
    state.doString("function multiRet() return 4.5, 10, false, 6.5 end");
    double doubleValue;
    float floatValue;
    lua::tie(doubleValue, intValue, boolValue, floatValue) = state["multiRet"]();
    
    check(state.flushStack(), 0);
    
    return 0;
}
