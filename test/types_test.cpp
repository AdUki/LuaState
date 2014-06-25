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
    // We create state and variables
    lua::State state;
    state.doString(createVariables);
    
    // Boolean
    bool boolValue = false;
    
    assert(state["boolean"] == true);
    assert((boolValue = state["boolean"]) == true);
    
    // All numeric types
    int                 intValue;
    long                longValue;
    long long           longlongValue;
    unsigned            unsignedValue;
    long unsigned       unsignedLongValue;
    long long unsigned  unsignedLongLongValue;
    short               shortValue;
    unsigned short      unsignedShortValue;
    
    state.set("value", 1);
    assert(state["value"] == 1);
    state.set("value", 1L);
    assert(state["value"] == 1);
    state.set("value", 1LL);
    assert(state["value"] == 1);
    state.set("value", 1U);
    assert(state["value"] == 1);
    state.set("value", 1LU);
    assert(state["value"] == 1);
    state.set("value", 1LLU);
    assert(state["value"] == 1);
    state.set("value", static_cast<short>(1));
    assert(state["value"] == 1);
    state.set("value", static_cast<short>(1));
    assert(state["value"] == 1);

    assert(state["integer"] == 10);
    assert(state["integer"] != 5);
    assert(state["integer"] > 9);
    assert(state["integer"] < 11);
    assert(state["integer"] >= 10);
    assert(state["integer"] <= 10);
    
    assert(state["integer"] == 10L);
    assert(state["integer"] != 5L);
    assert(state["integer"] > 9L);
    assert(state["integer"] < 11L);
    assert(state["integer"] >= 10L);
    assert(state["integer"] <= 10L);
    
    assert(state["integer"] == 10LL);
    assert(state["integer"] != 5LL);
    assert(state["integer"] > 9LL);
    assert(state["integer"] < 11LL);
    assert(state["integer"] >= 10LL);
    assert(state["integer"] <= 10LL);
    
    assert(state["integer"] == 10U);
    assert(state["integer"] != 5U);
    assert(state["integer"] > 9U);
    assert(state["integer"] < 11U);
    assert(state["integer"] >= 10U);
    assert(state["integer"] <= 10U);

    assert(state["integer"] == 10UL);
    assert(state["integer"] != 5UL);
    assert(state["integer"] > 9UL);
    assert(state["integer"] < 11UL);
    assert(state["integer"] >= 10UL);
    assert(state["integer"] <= 10UL);
    
    assert(state["integer"] == 10ULL);
    assert(state["integer"] != 5ULL);
    assert(state["integer"] > 9ULL);
    assert(state["integer"] < 11ULL);
    assert(state["integer"] >= 10ULL);
    assert(state["integer"] <= 10ULL);
    
    assert(state["integer"] == static_cast<short>(10));
    assert(state["integer"] != static_cast<short>(5));
    assert(state["integer"] > static_cast<short>(9));
    assert(state["integer"] < static_cast<short>(11));
    assert(state["integer"] >= static_cast<short>(10));
    assert(state["integer"] <= static_cast<short>(10));
    
    assert(state["integer"] == static_cast<unsigned short>(10));
    assert(state["integer"] != static_cast<unsigned short>(5));
    assert(state["integer"] > static_cast<unsigned short>(9));
    assert(state["integer"] < static_cast<unsigned short>(11));
    assert(state["integer"] >= static_cast<unsigned short>(10));
    assert(state["integer"] <= static_cast<unsigned short>(10));
    

    assert((intValue = state["integer"]) == 10);
    assert((intValue = state["integer"]) != 5);
    assert((intValue = state["integer"]) > 9);
    assert((intValue = state["integer"]) < 11);
    assert((intValue = state["integer"]) >= 10);
    assert((intValue = state["integer"]) <= 10);
    
    assert((longValue = state["integer"]) == 10L);
    assert((longValue = state["integer"]) != 5L);
    assert((longValue = state["integer"]) > 9L);
    assert((longValue = state["integer"]) < 11L);
    assert((longValue = state["integer"]) >= 10L);
    assert((longValue = state["integer"]) <= 10L);
    
    assert((longlongValue = state["integer"]) == 10LL);
    assert((longlongValue = state["integer"]) != 5LL);
    assert((longlongValue = state["integer"]) > 9LL);
    assert((longlongValue = state["integer"]) < 11LL);
    assert((longlongValue = state["integer"]) >= 10LL);
    assert((longlongValue = state["integer"]) <= 10LL);
    
    assert((unsignedValue = state["integer"]) == 10U);
    assert((unsignedValue = state["integer"]) != 5U);
    assert((unsignedValue = state["integer"]) > 9U);
    assert((unsignedValue = state["integer"]) < 11U);
    assert((unsignedValue = state["integer"]) >= 10U);
    assert((unsignedValue = state["integer"]) <= 10U);
    
    assert((unsignedLongValue = state["integer"]) == 10UL);
    assert((unsignedLongValue = state["integer"]) != 5UL);
    assert((unsignedLongValue = state["integer"]) > 9UL);
    assert((unsignedLongValue = state["integer"]) < 11UL);
    assert((unsignedLongValue = state["integer"]) >= 10UL);
    assert((unsignedLongValue = state["integer"]) <= 10UL);

    assert((unsignedLongLongValue = state["integer"]) == 10ULL);
    assert((unsignedLongLongValue = state["integer"]) != 5ULL);
    assert((unsignedLongLongValue = state["integer"]) > 9ULL);
    assert((unsignedLongLongValue = state["integer"]) < 11ULL);
    assert((unsignedLongLongValue = state["integer"]) >= 10ULL);
    assert((unsignedLongLongValue = state["integer"]) <= 10ULL);
    
    assert((shortValue = state["integer"]) == 10);
    assert((shortValue = state["integer"]) != 5);
    assert((shortValue = state["integer"]) > 9);
    assert((shortValue = state["integer"]) < 11);
    assert((shortValue = state["integer"]) >= 10);
    assert((shortValue = state["integer"]) <= 10);
    
    assert((unsignedShortValue = state["integer"]) == 10);
    assert((unsignedShortValue = state["integer"]) != 5);
    assert((unsignedShortValue = state["integer"]) > 9);
    assert((unsignedShortValue = state["integer"]) < 11);
    assert((unsignedShortValue = state["integer"]) >= 10);
    assert((unsignedShortValue = state["integer"]) <= 10);
    
    // All floating types
    float       floatValue;
    double      doubleValue;
    long double longDoubleValue;
    
    state.set("value", 1.0);
    assert(state["value"] == 1);
    state.set("value", 1.f);
    assert(state["value"] == 1);
    state.set("value", 1.l);
    assert(state["value"] == 1);
    
    assert(state["number"] == 2.5);
    assert(state["number"] != 3.5);
    assert(state["number"] > 2.0);
    assert(state["number"] < 3.0);
    assert(state["number"] >= 2.5);
    assert(state["number"] <= 3.5);

    assert(state["number"] == 2.5f);
    assert(state["number"] != 3.5f);
    assert(state["number"] > 2.f);
    assert(state["number"] < 3.f);
    assert(state["number"] >= 2.5f);
    assert(state["number"] <= 3.5f);

    assert(state["number"] == 2.5l);
    assert(state["number"] != 3.5l);
    assert(state["number"] > 2.l);
    assert(state["number"] < 3.l);
    assert(state["number"] >= 2.5l);
    assert(state["number"] <= 3.5l);


    assert((doubleValue = state["number"]) == 2.5);
    assert((doubleValue = state["number"]) != 3.5);
    assert((doubleValue = state["number"]) > 2.0);
    assert((doubleValue = state["number"]) < 3.0);
    assert((doubleValue = state["number"]) >= 2.5);
    assert((doubleValue = state["number"]) <= 3.5);

    assert((floatValue = state["number"]) == 2.5f);
    assert((floatValue = state["number"]) != 3.5f);
    assert((floatValue = state["number"]) > 2.f);
    assert((floatValue = state["number"]) < 3.f);
    assert((floatValue = state["number"]) >= 2.5f);
    assert((floatValue = state["number"]) <= 3.5f);

    assert((longDoubleValue = state["number"]) == 2.5l);
    assert((longDoubleValue = state["number"]) != 3.5l);
    assert((longDoubleValue = state["number"]) > 2.l);
    assert((longDoubleValue = state["number"]) < 3.l);
    assert((longDoubleValue = state["number"]) >= 2.5l);
    assert((longDoubleValue = state["number"]) <= 3.5l);
    
    // All character types
    char             charValue;
//    signed char      signedCharValue;
    unsigned char    unsignedCharValue;
//    wchar_t          wcharValue;
//    char16_t         char16Value;
//    char32_t         char32Value;
    
    const char*            charString;
//    const signed char*     signedCharString;
    const unsigned char*   unsignedCharString;
//    const wchar_t*         wcharString;
//    const char16_t*        char16String;
//    const char32_t*        char32String;
    
    state.set("value", 'x');
    assert(state["value"] == 'x');
    state.set("value", "ahoj");
    assert(strcmp(state["value"], "ahoj") == 0);
    
    assert(state["char"] == 'a');
    assert(state["char"] != 'b');
    assert((charValue = state["char"]) == 'a');
    assert((charValue = state["char"]) != 'b');

    unsignedCharValue = 'a';
    assert(state["char"] == unsignedCharValue);
    unsignedCharValue = 'b';
    assert(state["char"] != 'b');
    assert((unsignedCharValue = state["char"]) == 'a');
    assert((unsignedCharValue = state["char"]) != 'b');
    
    assert(strcmp(state["text"], "hello") == 0);
    assert(strcmp(state["text"], "bannana") != 0);
    assert(strcmp(charString = state["text"], "hello") == 0);
    assert(strcmp(charString = state["text"], "bannana") != 0);
    
    unsignedCharString = state["text"];
    assert(unsignedCharString[0] == 'h');
    assert(unsignedCharString[1] == 'e');
    assert(unsignedCharString[2] == 'l');
    assert(unsignedCharString[3] == 'l');
    assert(unsignedCharString[4] == 'o');
    assert(unsignedCharString[5] == '\0');
    
    std::string stringValue = "test string";
    state.set("value", stringValue);
    assert(state["value"] == stringValue);
    assert((stringValue = state["text"].toString()) == "hello");
    assert((stringValue = state["text"].toString()) != "bannana");
    
    char binaryData[3];
    binaryData[0] = 'a';
    binaryData[1] = 'b';
    binaryData[2] = 'c';
    state.setData("binary", binaryData, 3);
    assert(strcmp(state["binary"], "abc") == 0);
    
    state.checkMemLeaks();
    return 0;
}
