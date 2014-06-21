[![Build Status](https://travis-ci.org/AdUki/LuaState.png)](https://travis-ci.org/AdUki/LuaState)

LuaState
========

Lightweight Lua51 binding library for C++11.

If you find bugs, feel free to create new issues. Also I will gladly to hear new suggestions and improvements :)

#### Main features:
 * While getting values from `lua_State*` library does not use vectors or lists for memoization of key values. It uses values which are already in native Lua C API stack and simple int variables to index them.
 * To keep Lua C API stack clean. When we create lua_State*, library automaticaly creates priority queue, where we keep when and which Lua values can be removed from stack. This can be big performacne gain while querying deep structures from tables.
 * We can bind lamba functions, where captured variables are managed by Lua garbage collector.
 * No nesting C preprocessor macros to bind our classes. We can use only C++11 code and bind our classes with capture lists of lambdas. This way we can bind anything: function, variables, pointers...
 * When we can bind almost ANY function. There is no predefined form. In that way we can avoid boilerplate code and parameters can be self documented with their names.
 * Multiple return values with `std::tuples` - `lua::tie` (same syntax as `std::tie`).
 * We can directly pass lua values (tables for instance) around in C++ code. For example we can return table from one Lua function and put this table as parameter to another Lua function. All this just in C++.

### Setting it up

Just create lua::State variable, which will initialize lus_State and loads up standard libraries. It will close state automatically in destructor.

~~~~~~~~~~~~~~~{.cpp}
#include <luastate.h>
int main() {
   lua::State state;
	state.doString("print 'Hello world!'");
 }
~~~~~~~~~~~~~~~

### Reading values

Reading values from Lua state is very simple. It is using templates, so type information is required.

~~~~~~~~~~~~~~~{.cpp}
state.doString("number = 100; text = 'hello'");
int number =state["number"];
std::string text = state["text"];
~~~~~~~~~~~~~~~

When reading values from tables, you just chain [] operators.

~~~~~~~~~~~~~~~{.cpp}
state.doString("table = { a = 1, b = { 2 }, c = 3}");
int a = state["table"]["a"];
int b = state["table"]["b"][1];
int c = state["table"]["c"];
~~~~~~~~~~~~~~~

### Calling functions

You can call lua functions with () operator with various number of arguments while returning none, one or more values.

~~~~~~~~~~~~~~~{.cpp}
state.doString("function setFoo() foo = "hello" end");
state.doString("function getFoo() return foo end");

state["setFoo"]()
std::string text = state["getFoo"]()

state.doString("function add(x, y) return x + y end");
int result = state["add"](1,2);

state.doString("function iWantMore() return 20, 13.8, 'MORE' end");
float number;
std::string text;
lua::tie(result, number, text) = state["iWantMore"]();
~~~~~~~~~~~~~~~

### Setting values

Is also pretty straightforward...

~~~~~~~~~~~~~~~{.cpp}
state.doString("table = { a = 1, b = { 2 }, c = 3}");
state["table"].set("a", 100);
state["table"]["b"].set(1, 200);
state["table"].set("c", 300);

state.set("newTable", lua::Table());
state["newTable"].set(1, "a");
state["newTable"].set(2, "b");
state["newTable"].set(3, "c");
~~~~~~~~~~~~~~~

### Setting functions

You can bind C functions, lambdas and std::functions with bind. These instances are managed by Lua garbage collector and will be destroyed when you will lost last reference in Lua state to them.

~~~~~~~~~~~~~~~{.cpp}
void sayHello() { printf("Hello!\n"); }
state.set("cfunction", &sayHello);
state["cfunction"](); // Hello!

int value = 20;
state.set("lambda", [value](int a, int b) -> int { return (a*b)/value; } )
int result = state["lambda"](12, 5); // result = 3
~~~~~~~~~~~~~~~

They can return one or more values with use of std::tuple. For example, when you want to register more functions, you can return bundled in tuple...

~~~~~~~~~~~~~~~{.cpp}
state.set("getFncs", []()
-> std::tuple<std::function<int()>, std::function<int()>, std::function<int()>> {
    return {
    	[]() -> int { return 100; },
		[]() -> int { return 200; },
		[]() -> int { return 300; }
	};
} );
state.doString("fnc1, fnc2, fnc3 = getFncs()"
               "print(fnc1(), fnc2(), fnc3())"); // 100 200 300
~~~~~~~~~~~~~~~

You can easily register your classes functions with `this` pointer passing to lambda capture or bind...

~~~~~~~~~~~~~~~{.cpp}
struct Foo {
	int a; int b;

	void setB(int value) { b = value; }
	Foo(lua::State& state) {
        state.set("Foo_setA", [this](int value) { a = value; } );
        state.set("Foo_setB", std::function<void(int)>(std::bind(&Foo::setB, this, _1)) );
	}
};
~~~~~~~~~~~~~~~

### Managing C++ classes by garbage collector

It is highly recommended to use shared pointers and then you will have garbage collected classes in C++. Objects will exist util there is last instance of shared pointer and they will be immediately released when all shared pointer instances are gone.

Our resource:

~~~~~~~~~~~~~~~{.cpp}
struct Resource {
    Resource() { printf("New resource\n"); }
    ~Resource() { printf("Released resource\n"); }

    void doStuff() { printf("Working..."); }
};
~~~~~~~~~~~~~~~

Resource using and released by garbage collector:

~~~~~~~~~~~~~~~{.cpp}
std::shared_ptr<Resource> resource = std::make_shared<Resource>(); // New resource
state.set("useResource", [resource]() { resource->doStuff(); } );
resource.reset();

state.doString("useResource()"); // Working
state.doString("useResource = nil; collectgarbage()"); // Released resource
~~~~~~~~~~~~~~~
