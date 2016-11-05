Dukglue
=======

A C++ extension to the embeddable Javascript engine [Duktape](http://duktape.org/).

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


What Dukglue **doesn't do:**

* Dukglue does not support automatic garbage collection of C++ objects. Why?

    To support objects going back and forth between C++ and script, Dukglue keeps one canonical script object per native object (which is saved in the Duktape 'heap stash'). The downside to this is that Dukglue is always holding onto script object references, so they can't be garbage collected.

    If Dukglue *didn't* keep a registry of native object -> canonical script object, you could have two different script objects with the same underlying native object, but different dynamic properties. Equality operators like `obj1 == obj2` would also be unreliable, since they would technically be different objects.

    (You *could* keep dynamic properties consistent by using a [Proxy object](http://wiki.duktape.org/HowtoVirtualProperties.html), but you still couldn't compare objects for equality with `==`.)

    Basically, for this to be possible, Duktape needs support for weak references, so Dukglue can keep references without keeping them from being garbage collected.

* Dukglue *might* not follow the "compact footprint" goal of Duktape. I picked Duktape for it's simple API, not to script my toaster. YMMV if you're trying to compile this for a microcontroller. Why?

    * Dukglue currently needs RTTI turned on. When Dukglue checks if an object can be cast to a particular type, it uses the typeid operator to compare if two types are equal. It's always used on compile-time types though, so you could implement it without RTTI if you needed to. I believe this is the only place RTTI is used (and a smart compiler should do it at compile-time).

    * An std::unordered_map is used to efficiently map object pointers to an internal Duktape array (see the `RefMap` class in `detail_refs.h`). This has some unnecessary memory overhead for every object (probably around 32 bytes per object). This could be improved to have no memory overhead - see detail_refs.h's comments for more information.

    * That aside, run-time memory usage should be reasonable.

* Dukglue may not be super fast. Duktape doesn't promise to be either.

Getting Started
===============

Dukglue has been tested with MSVC 2015, clang++-3.6, and g++-4.8. Your compiler must support at least C++11 to use Dukglue.

Dukglue is a header-only library. Dukglue requires Duktape to be installed such that `#include <duktape.h>` works.

Everything you need is in the `dukglue` directory of this repository. If you prefer not to mess with CMake or build settings, you should be able to just copy the `dukglue` directory into your project.

I've also created some CMake files to try and make installing Dukglue as painless as possible:

```bash
mkdir build
cd build
cmake ..
sudo make install
```

This will copy the Dukglue headers to `/usr/local/include/dukglue/*` by default. You can use `cmake .. -DCMAKE_INSTALL_PREFIX:PATH=.` to change the install directory to something else (will install to `CMAKE_INSTALL_PREFIX/include/dukglue/*`).

Now, all you need to do is add the include paths for Dukglue to your project and add duktape.c/duktape.h to your project.

If your project is using CMake, feel free to use the FindDukglue.cmake in `cmake_modules`. It assumes you will include Duktape directly in your project. You can use it like this:

```cmake
cmake_minimum_required(VERSION 3.1.0)

project(MyAwesomeProject)

find_package(Dukglue REQUIRED)

add_executable(MyAwesomeProject
  main.cpp
  # ... your files ...
  duktape.c
  duktape.h
  duk_config.h
)

include_directories(${DUKGLUE_INCLUDE_DIR})

# Enable C++11 mode for your compiler, if you don't do this you will get crazy errors
target_compile_features(MyAwesomeProject PRIVATE cxx_variadic_templates cxx_auto_type)
```

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

* Add helpers for calling script functions and handling the values that come out

* write a C++ wrapper for duk_context that has member functions that forward to the dukglue_* functions

* write a tutorial on how to add custom value types

http://aloshi.com

Alec 'Aloshi' Lofquist
