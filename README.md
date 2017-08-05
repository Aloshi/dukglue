Dukglue
=======

A C++ extension to the embeddable Javascript engine [Duktape](http://duktape.org/).

NOTE: The master branch is for Duktape 2.x. Check the `duktape_1.x` branch if you want to use the older Duktape 1.x.

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

```cpp
// C++:
struct Point {
  Point(float x_in, float y_in) : x(x_in), y(y_in) {}

  float x;
  float y;
}

float calcDistance(Point* p1, Point* p2) {
  float x = (p2->x - p1->x);
  float y = (p2->y - p1->y);
  return sqrt(x*x + y*y);
}

dukglue_register_constructor<Point, /* constructor args */ float, float>(ctx, "Point");
dukglue_register_function(ctx, calcDistance, "calcDistance");


class Apple {
  void eat();
}

dukglue_register_constructor<Apple>(ctx, "Apple");

// --------------

// Script:
var p1 = new Point(0, 0);
var p2 = new Point(5, 0);
var apple = new Apple();
print(calcDistance(p1, p2));  // prints 5
calcDistance(apple, p2);  // throws an error
```

* You can even return new C++ objects from C++ functions/methods:

```cpp
// C++:
class Dog {
  Dog(const char* name) : name_(name) {}

  const char* getName() {
    return name_.c_str();
  }

private:
  std::string name_;
}

Dog* adoptPuppy() {
  return new Dog("Gus");
}

void showFriends(Dog* puppy) {
  if (puppy != NULL)
    std::cout << "SO CUTE" << std::endl;
  else
    std::cout << "why did you call me, there is nothing cute here" << std::endl;
}

dukglue_register_function(ctx, adoptPuppy, "adoptPuppy");
dukglue_register_function(ctx, showFriends, "showFriends");

// Notice we never explicitly told Duktape about the "Dog" class!

// --------------

// Script:
var puppy = adoptPuppy();
showFriends(puppy);  // prints "SO CUTE" via std::cout
showFriends(null);  // prints "why did you call me, there is nothing cute here"
```

* You can invalidate C++ objects when they are destroyed:

```cpp
// C++:
void puppyRanAway(Dog* dog) {
  delete dog;
  dukglue_invalidate_object(ctx, dog);  // tell Duktape this reference is now invalid
  cry();

  // (note: you'll need access to the Duktape context, e.g. as a global/singleton, for this to work)
}

dukglue_register_function(ctx, puppyRanAway, "puppyRanAway");

// --------------

// Script:
var puppy = adoptPuppy();
puppyRanAway(puppy);
print(puppy.getName());  // puppy has been invalidated, methods throw an error
showFriends(puppy);  // also throws an error, puppy has been invalidated
```

* Dukglue also works with single inheritance:

```cpp
// C++:

class Shape {
public:
  Shape(float x, float y) : x_(x), y_(y) {}

  virtual void describe() = 0;

protected:
  float x_, y_;
}

class Circle : public Shape {
  Circle(float x, float y, float radius) : Shape(x, y), radius_(radius) {}

  virtual void describe() override {
    std::cout << "A lazily-drawn circle at " << x_ << ", " << y_ << ", with a radius of about " << radius_ << std::endl;
  }

protected:
  float radius_;
}

dukglue_register_method(ctx, &Shape::describe, "describe");

dukglue_register_constructor<Circle, float, float, float>(ctx, "Circle");
dukglue_set_base_class<Shape, Circle>(ctx);

// --------------

// Script:
var shape = new Circle(1, 2, 42);
shape.describe();  // prints "A lazily-drawn circle at 1, 2, with a radius of about 42"
```

  (multiple inheritance is not supported)

* Dukglue supports Duktape properties (getter/setter pairs that act like values):

```cpp
class MyClass {
public:
  MyClass() : mValue(0) {}

  int getValue() const {
    return mValue;
  }
  void setValue(int v) {
    mValue = v;
  }

private:
  int mValue;
};

dukglue_register_constructor<MyClass>(ctx, "MyClass");
dukglue_register_property(ctx, &MyClass::getValue, &MyClass::setValue, "value");

// --------------

// Script:
var test = MyClass();
test.value;  // calls MyClass::getValue(), which returns 0
test.value = 42;  // calls MyClass::setValue(42)
test.value;  // again calls MyClass::getValue(), which now 42
```

  (also works with non-`const` getter methods)

* You can also do getter-only or setter-only properties:

```cpp
// continuing with the class above...

dukglue_register_property(ctx, &MyClass::getValue, nullptr, "value");

var test = MyClass();
test.value;  // still 0
test.value = 42;  // throws an error

dukglue_register_property(ctx, nullptr, &MyClass::setValue, "value");
test.value = 42;  // works
test.value;  // throws an error
```

  (it is also safe to re-define properties like in this example)

* There are utility functions for pushing arbitrary values onto the Duktape stack:

```cpp
// you can push primitive types
int someValue = 12;
dukglue_push(ctx, someValue);

// you can push native C++ objects
Dog* myDog = new Dog("Zoey");
dukglue_push(ctx, myDog);  // pushes canonical script object for myDog

// you can push multiple values at once
dukglue_push(ctx, 12, myDog);  // stack now contains "12" at position -2 and "myDog" at -1
```

* There is a utility function for doing a `duk_peval` and getting the return value safely:

```cpp
// template argument is the return value we expect
int result = dukglue_peval<int>(ctx, "4 * 5");
result == 20;  // true
duk_get_top(ctx) == 0;  // the Duktape stack is empty, return value is always popped off the stack

int error = dukglue_peval<int>(ctx, "'a horse-sized duck'");  // string is not a number; throws a DukException

// we can also choose to ignore the return value
dukglue_peval<void>(ctx, "function makingAFunction() { doThings(); }");
// the Duktape stack will be clean
```

* There is a helper function for registering script objects as globals (useful for singletons):

```cpp
Dog* myDog = new Dog("Zoey");
dukglue_register_global(ctx, myDog, "testDog");
// testDog in script now refers to myDog in C++
// use dukglue_invalidate_object(ctx, myDog) to invalidate it

// --------------
// Script:
testDog.getName() == "Zoey";  // this evaluates to true
```

* You can call script methods on native objects (useful for callbacks that are defined as properties):

```cpp
// continuing from the above example...
// Script:
testDog.barkAt = function (at) { print(this.getName() + " barks at " + at +"!"); }

// --------------
// C++:
// template parameter is required, and is the desired return type for the method
dukglue_pcall_method<void>(ctx, myDog, "barkAt", "absolutely nothing");  // prints "Zoey barks at absolutely nothing!"
```

* We can get return values from methods:

```cpp
// Script:
testDog.checkWantsTreat = function() { return true; }  // frequency chosen via empirical evidence

// --------------
// C++:
bool wantsTreatNow = dukglue_pcall_method<bool>(ctx, myDog, "checkWantsTreat");  // true

// return types are typechecked, so the following is an error (reported via exception):
std::string error = dukglue_pcall_method<std::string>(ctx, myDog, "checkWantsTreat");  // throws DukException (inherits std::exception)

// we can ignore the return value (whatever it is)
dukglue_pcall_method<void>(ctx, myDog, "checkWantsTreat");
```


* You can get/persist references to script values using the `DukValue` class:

```cpp
// template parameter is return type
DukValue testObj = dukglue_peval<DukValue>(ctx,
  "var testObj = new Object();"
  "testObj.value = 42;"
  "testObj.myFunc = function(a, b) { return a*b; };"
  "testObj;");  // returns testObj

// testObj now holds a reference to the testObj we made in script above
// we can call methods on it:
{
  int result = dukglue_pcall_method<int>(ctx, testObj, "myFunc", 3, 4);
  result == 12;  // true
}

// if we don't know what the return type will be, we can use a DukValue:
{
  DukValue result = dukglue_pcall_method<DukValue>(ctx, testObj, "myFunc", 5, 4);
  if (result.type() == DukValue::NUMBER)
    std::cout << "Returned " << result.as_int() << "\n";
  if (result.type() == DukValue::UNDEFINED)
    std::cout << "Didn't return anything\n";
}

// we can also have DukValue hold a script function
DukValue printValueFunc = dukglue_peval<DukValue>(ctx,
  "function printValueProperty(obj) { print(obj.value); };"
  "printValueProperty;");

// we can use duk_pcall to call a callable DukValue (i.e. a function)
// and we can also use a DukValue as a parameter to another function
dukglue_pcall(ctx, printValueFunc, testObj);  // prints 42

// we can copy DukValues if we want:
DukValue printCopy = printValueFunc;
printCopy == printValueFunc;  // true
// since printCopy.type() == OBJECT, both values will reference the same object
// (i.e. changing printCopy with also change printValueFunc)
// DukValues are reference counted, you don't need to worry about manually freeing them!

// even if we make a totally new DukValue, it will still reference the same script object
// (and the equality operator still works):
DukValue anotherPrintValueFunc = dukglue_peval<DukValue>(ctx, "printValueProperty;");
anotherPrintValueFunc == printValueFunc;  // true
```

  (a DukValue can hold any Duktape value except for buffer and lightfunc)
  (C++ getters for type, null, boolean, number, string, and pointer - no getter for native objects yet, though this is definitely possible)


What Dukglue **doesn't do:**

* Dukglue does not support automatic garbage collection of C++ objects. Why?

    To support objects going back and forth between C++ and script, Dukglue keeps one canonical script object per native object (which is saved in the Duktape 'heap stash'). The downside to this is that Dukglue is always holding onto script object references, so they can't be garbage collected.

    If Dukglue *didn't* keep a registry of native object -> canonical script object, you could have two different script objects with the same underlying native object, but different dynamic properties. Equality operators like `obj1 == obj2` would also be unreliable, since they would technically be different objects.

    (You *could* keep dynamic properties consistent by using a [Proxy object](http://wiki.duktape.org/HowtoVirtualProperties.html), but you still couldn't compare objects for equality with `==`.)

    Basically, for this to be possible, Duktape needs support for weak references, so Dukglue can keep references without keeping them from being garbage collected.

* Dukglue supports `std::shared_ptr`, but with two major caveats:

    1. **Dynamic properties will not persist** - any properties not defined with `dukglue_register_property` will not be the same between two shared_ptrs pointing to the same object. For example:

    ```cpp
    std::shared_ptr<Resource> getResource() {
      static std::shared_ptr<Resource> resource = std::make_shared<Resource>();
      return resource;
    }
    ```

    ```js
    var resource = getResource();
    tex.isAwesome = true;
    var resourceAgain = getResource();
    print(resourceAgain.isAwesome);  // prints undefined
    print(tex.isAwesome);  // prints true
    ```

    (However, dynamic properties will stay on the object once it has entered script until it is garbage collected - `tex.isAwesome` will still be `true`.)

    One minor upside to properties not persisting is that **std::shared_ptr objects don't need to call `dukglue_invalidate_reference()` when they are destroyed**.
    
    2. **JavaScript equality checks will not work**. This is because the current implementation treats shared_ptrs like a value type. 

    ```js
    print(tex === texAgain);  // prints false
    ```

    I'm not really happy with these caveats, but it was the fastest way to implement and it is acceptable for my use case. The entire implementation is in detail_primitive_types.h if you want to try and improve it (maybe try using Duktape's ES6 proxy subset?).

* Dukglue *might* not follow the "compact footprint" goal of Duktape. I picked Duktape for it's simple API, not to script my toaster. YMMV if you're trying to compile this for a microcontroller. Why?

    * Dukglue currently needs RTTI turned on. When Dukglue checks if an object can be cast to a particular type, it uses the typeid operator to compare if two types are equal. It's always used on compile-time types though, so you could implement it without RTTI if you needed to. Dukglue also uses exceptions in two places: the `dukglue_pcall*` functions (since these return a value instead of an error code, unlike Duktape), and the `DukValue` class (to communicate type errors on getters and unsupported types).

    * An std::unordered_map is used to efficiently map object pointers to an internal Duktape array (see the `RefMap` class in `detail_refs.h`). This has some unnecessary memory overhead for every object (probably around 32 bytes per object). This could be improved to have no memory overhead - see detail_refs.h's comments for more information.

    * That aside, run-time memory usage should be reasonable.

* Dukglue may not be super fast. Duktape doesn't promise to be either.

Getting Started
===============

Dukglue has been tested with MSVC 2015, clang++-3.8, and g++-5.4. Your compiler must support at least C++11 to use Dukglue.

Dukglue is a header-only library. Dukglue requires Duktape to be installed such that `#include <duktape.h>` works.

Everything you need (except Duktape) is in the `include` directory of this repository. If you prefer not to mess with CMake or build settings, you should be able to just copy the `include` directory into your project.

I've also created some CMake files to try and make installing Dukglue as painless as possible:

```bash
mkdir build
cd build
cmake ..
sudo make install
```

This will copy the Dukglue headers to `/usr/local/include/dukglue/*` by default. You can use `cmake .. -DCMAKE_INSTALL_PREFIX:PATH=.` to change the install directory to something else (will install to `CMAKE_INSTALL_PREFIX/include/dukglue/*`).

Now, all you need to do is add the include paths for Dukglue to your project and add duktape.c/duktape.h to your project.

Then you can try the example below:

```cpp
#include <iostream>

#include <dukglue/dukglue.h>

int myFunc(int a)
{
  return a*a;
}

class Dog {
public:
  Dog(const char* name) : name_(name) {}

  void bark() {
    std::cout << "WOOF WOOF" << std::endl;
  }

  const char* getName() {
    return name_.c_str();
  }

private:
  std::string name_;
}

int main()
{
  duk_context* ctx = duk_create_heap_default();

  dukglue_register_function(ctx, myFunc, "myFunc");

  dukglue_register_constructor<Dog, /* constructor args: */ const char*>(ctx, "Dog");
  dukglue_register_method(ctx, &Dog::bark, "bark");
  dukglue_register_method(ctx, &Dog::getName, "getName");

  if (duk_peval_string(ctx,
      "var gus = new Dog('Gus');"
      "gus.bark();"
      "print(gus.getName());")) {
    // if an error occured while executing, print the stack trace
    duk_get_prop_string(ctx, -1, "stack");
    std::cout << duk_safe_to_string(ctx, -1) << std::endl;
    duk_pop(ctx);
  }

  duk_destroy_heap(ctx);

  return 0;
}
```

TODO
====

* Make better organized documentation

* Write a tutorial on how to add custom value types

http://aloshi.com

Alec 'Aloshi' Lofquist
