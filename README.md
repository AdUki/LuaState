LuaState
========

Lightweight Lua51 binding library for C++11.

### Setting it up

Just create lua::State variable, which will initialize lus_State and loads up standard libraries. It will close state automatically in destructor.

``` cpp
int main() {
   lua::State state;
	state.doString("print 'Hello world!'");
 }
```

### Reading values

Reading values from Lua state is very simple. It is using templates, so type information is required.

```cpp
state.doString("number = 100; text = 'hello'");
int number =state["number"];
std::string text = state["text"];
```

When reading values from tables, you just chain [] operators.

```cpp
state.doString("table = { a = 1, b = { 2 }, c = 3}");
int a = state["table"]["a"];
int b = state["table"]["b"][1];
int c = state["table"]["c"];
```

### Calling functions

You can call lua functions with () operator with various number of arguments while returning none, one or more values.

```cpp
state.doString("function setFoo() foo = "hello" end");
state.doString("function getFoo() return foo end");

state["setFoo"]()
std::string text = state["getFoo"]()

state.doString("function add(x, y) return x + y end");

int result = state["add"](1,2);
```

### Setting values
 
Is also pretty straightforward...

```cpp
state.doString("table = { a = 1, b = { 2 }, c = 3}");
state["table"]["a"] = 100;
state["table"]["b"][1] = 200;
state["table"]["c"] = 300;

state["newTable"] = lua::Table();
state["newTable"][1] = "a";
state["newTable"][2] = "b";
state["newTable"][3] = "c";
```

### Setting funcitons

You can bind C functions, lambdas and std::functions with bind. These instances are managed by Lua garbage collector and will be destroyed when you will lost last reference in Lua state to them.

```cpp
void sayHello() { printf("Hello!\n"); }
state["cfunction"] = &sayHello;
state["cfunction"](); // Hello!

int value = 20;
state["lambda"] = [value](int a, int b) -> int { return (a*b)/value; }
int result = state["lambda"](12, 5); // result = 3
```
