Dukglue
=======

A C++ extension to the wonderful [Duktape](http://duktape.org/) embeddable Javascript engine.

Dukglue offers:

* An easy, type-safe way to bind functions:

```cpp
// C++:
bool is_mod_2(int a)
{
  return (a % 2) == 0;
}

dukglue_register_function(ctx, &is_mod_2, "is_mod_2");

// --------------

// Script:

is_mod_2(5);  // returns false
is_mod_2("a string!");  // throws an error
```

* An easy, type-safe way to use C++ objects in scripts:

```cpp
// C++:
class TestClass
{
public:
  TestClass() : mCounter(0) {}

  void incCounter(int val) {
    mCounter += val;
  }

  void printCounter() {
    std::cout << "Counter: " << mCounter << std::endl;
  }

private:
  int mCounter;
};

dukglue_register_constructor<TestClass>(ctx, "TestClass");
dukglue_register_method(ctx, &TestClass::incCounter, "incCounter");
dukglue_register_method(ctx, &TestClass::printCounter, "printCounter");

// --------------

// Script:

var test = new TestClass();
test.incCounter(1);
test.printCounter();  // prints "Counter: 1" via std::cout
test.incCounter("a string!")  // throws an error (TestClass::incCounter expects a number)
```

* You can safely call C++ functions that take pointers as arguments:

// TODO

* You can even return new C++ objects from C++ functions:

// TODO

* Dukglue also behaves well with single inheritance:

  (multiple inheritance is not supported)

What Dukglue **doesn't do:**

* Dukglue does not support automatic garbage collection of C++ objects. Why?

    * To support objects going back and forth between C++ and script, Dukglue keeps one canonical script object per native object (which is saved in the Duktape 'heap stash'). Here's why this is important:

    ```cpp
    // global variable
    TestClass* test = new TestClass();

    // this is a bit contrived, but imagine any function that returns a C++ object
    TestClass* getGlobalObject() {
      return test;
    }

    dukglue_register_function(ctx, getGlobalObject, "getGlobalObject");

    // --------------

    // Script:

    getGlobalObject().some_script_variable = 5;
    Duktape.gc();
    print(getGlobalObject().some_script_variable);
    ```

    If Dukglue *didn't* keep a registry of native object -> canonical script object, this would print "undefined," because the script object would get destroyed and re-created.

    (You could also keep dynamic properties consistent by using a [Proxy object](http://wiki.duktape.org/HowtoVirtualProperties.html), but you then couldn't compare objects for equality with `==` anymore (since they would technically be two different script objects).)

    * For this to work with `std::shared_ptr`, the registry needs to keep *weak references* to the script objects so it doesn't prevent them from being garbage collected - but Duktape does not currently support weak references.

* Dukglue does not promise to follow the "compact footprint" goal of Duktape. I picked Duktape for it's simple API. YMMV if you're trying to compile this for a microcontroller.