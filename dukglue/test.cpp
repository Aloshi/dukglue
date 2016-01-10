#include <iostream>
#include <ctime>

#include "dukglue.h"

#include "duktape/duktape.h"

#include "example_vector3.h"

void timeTest(const char* name, duk_context* ctx, const char* test_string)
{
	static const int NUM_TESTS = 100;

	double total = 0;
	for (int i = 0; i < NUM_TESTS; i++) {
		const std::clock_t start = std::clock();
		if (duk_peval_string(ctx, test_string) != 0) {
			std::cout << "Error in test '" << name << "': " << duk_safe_to_string(ctx, -1) << std::endl;
			duk_pop(ctx);
			return;
		}

		const std::clock_t end = std::clock();

		double time = ((end - start) / (double)(CLOCKS_PER_SEC / 1000));
		total += time;
	}

	std::cout << "Test '" << name << "' took " << (total / NUM_TESTS) << "ms" << std::endl;
}

/*
double testFunc(double a, bool op)
{
	if (op)
		return a * a;
	else
		return a + a;
}

double testFunc2(double a, bool op)
{
	if (op)
		return a + a;
	else
		return a * a;
}

int main()
{
	duk_context* ctx = duk_create_heap_default();

	// ct
	dukglue_register_function_compiletime<decltype(testFunc), testFunc>(ctx, "testFunc1_ct", testFunc);
	dukglue_register_function_compiletime<decltype(testFunc), testFunc>(ctx, "testFunc2_ct", testFunc2);

	// rt
	dukglue_register_function(ctx, "testFunc1_rt", testFunc);
	dukglue_register_function(ctx, "testFunc2_rt", testFunc);

	timeTest("Compile-time generated function calls", ctx,
		"function main() {"
			"for (var i = 0; i < 10000; i++) {\n"
				"var test = Math.random() * 10 + Math.random() * 10;\n"
				"test = test * test;\n"
				"if (Math.random() > 0.5) {\n"
					"testFunc1_ct(test, Math.random() > 0.5);\n"
				"} else {\n"
					"testFunc2_ct(test, Math.random() > 0.5);\n"
				"}"
			"}"
		"}"
		"main();"
	);
	
	timeTest("Run-time lookup function calls", ctx,
		"function main() {"
			"for (var i = 0; i < 10000; i++) {\n"
				"var test = Math.random() * 10 + Math.random() * 10;\n"
				"test = test * test;\n"
				"if (Math.random() > 0.5) {\n"
					"testFunc1_rt(test, Math.random() > 0.5);\n"
				"} else {\n"
					"testFunc2_rt(test, Math.random() > 0.5);\n"
				"}"
			"}"
		"}"
		"main();"
	);

	//duk_peval_string(ctx, "testFunc_rt(4, false)");
	//std::cout << "Result: " << duk_require_number(ctx, -1) << std::endl;
	//duk_pop(ctx);

	duk_destroy_heap(ctx);

	while (true);
	return 0;
}
*/

/*
class TestClass
{
public:
	TestClass() : mCounter(0) {}

	inline void incCounter(int val) {
		mCounter += val;
	}

	inline void printCounter() {
		std::cout << "Counter: " << mCounter << std::endl;
	}

private:
	int mCounter;
};
*/

// Inheritance test

/*
class Shape
{
public:
	virtual int getNumber() = 0;
};

class Circle : public Shape
{
public:
	int getNumber() override {
		return 2;
	}
};
*/

// Native to script object test

/*
class Dog
{
public:
	Dog(const char* name) : mName(name) {}

	const char* name() const {
		return mName.c_str();
	}

	void rename(const char* newName) {
		mName = newName;
	}

	void bark() {
		std::cout << "WOOF WOOF MOTHERFUCKER" << std::endl;
	}
	
private:
	std::string mName;
};
*/

class Player {
public:
	Player(int id) : mID(id) {
		std::cout << "Created player with ID " << id << std::endl;
	}

	Vector3f getPosition() {
		return mPosition;
	}

	void setPosition(const Vector3f& position) {
		mPosition = position;
	}

	int getInt() const {
		return 42;
	}

	float getFloat() const {
		return 42.42f;
	}

	const char* getString() const {
		return "dicks";
	}

	void printIt(const char* str) const {
		std::cout << "printIt: " << str << std::endl;
	}

private:
	int mID;
	Vector3f mPosition;
};

int main()
{
	duk_context* ctx = duk_create_heap_default();

	//dukglue_register_constructor<Circle>(ctx, "Circle");
	//dukglue_register_method<Shape>(ctx, &Shape::getNumber, "getNumber");
	//dukglue_set_base_class<Shape, Circle>(ctx);

	//dukglue_register_function(ctx, deleteDog, "deleteDog");
	//dukglue_register_function(ctx, pokeWithStick, "pokeWithStick");

	//dukglue_register_constructor<Dog, const char*>(ctx, "Dog");
	//dukglue_register_method(ctx, &Dog::name, "name");
	//dukglue_register_method(ctx, &Dog::rename, "rename");
	// dukglue_register_function(ctx, pokeWithStick, "pokeWithStick");

	//dukglue_register_constructor<Player>(ctx, "Player");
	//dukglue_register_method(ctx, &Player::setPosition, "setPosition");
	//dukglue_register_method(ctx, &Player::getPosition, "getPosition");

	dukglue_register_constructor<Player, int>(ctx, "Player");
	//dukglue_register_method(ctx, &Player::getInt, "getInt");
	//dukglue_register_method(ctx, &Player::getFloat, "getFloat");
	dukglue_register_method(ctx, &Player::getString, "getString");
	dukglue_register_method(ctx, &Player::printIt, "printIt");

	std::cout << "Stack size after all registrations (should be 0): " << duk_get_top(ctx) << std::endl;
	std::cout << "-----------------" << std::endl;

	//dukglue_register_constructor<TestClass>(ctx, "TestClass");
	//dukglue_register_method(ctx, &TestClass::incCounter, "incCounter");
	//dukglue_register_method(ctx, &TestClass::printCounter, "printCounter");

	/*
	timeTest("Compile-time lookup method calls", ctx,
		"function main() {\n"
			"var obj = new TestClass();\n"
			"for (var i = 0; i < 6000; i++) {\n"
				"var test = Math.random() * 10 + Math.random() * 10;\n"
				"test = test * test;\n"
				"if (Math.random() > 0.5) {\n"
					"obj.testMethod1_ct(test, Math.random() > 0.5);\n"
				"} else {\n"
					"obj.testMethod2_ct(test, Math.random() > 0.5);\n"
				"}"
			"}"
		"}"
		"main();"
	);

	timeTest("Run-time lookup method calls", ctx,
		"function main() {\n"
			"var obj = new TestClass();\n"
			"for (var i = 0; i < 6000; i++) {\n"
				"var test = Math.random() * 10 + Math.random() * 10;\n"
				"test = test * test;\n"
				"if (Math.random() > 0.5) {\n"
					"obj.testMethod1_rt(test, Math.random() > 0.5);\n"
				"} else {\n"
					"obj.testMethod2_rt(test, Math.random() > 0.5);\n"
				"}"
			"}"
		"}"
		"main();"
	);
	*/

	//if (duk_peval_string(ctx, "var test = new TestClass(); test.incCounter(1); test.printCounter();")) {
	//if (duk_peval_string(ctx, "var test = getPuppy(); print(test.name()); test.rename('Archie'); print(getPuppy().name()); deletePuppy(); pokeWithStick(test);")) {
	if (duk_peval_string(ctx,
		"var pl = new Player(1); print(pl.getString()); pl.printIt('dogfarts');")) {
		duk_get_prop_string(ctx, -1, "stack");
		std::cout << duk_safe_to_string(ctx, -1) << std::endl;
		duk_pop(ctx);
	}

	duk_destroy_heap(ctx);

	while (true);
	return 0;
}
