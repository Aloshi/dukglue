#include "test_assert.h"
#include <dukglue/dukglue.h>

#include <iostream>

// no return, no arguments (empty argument tuple, needs no stack access)
void test_no_args() {

}

// read a single value type
void test_one_arg(int a) {
	std::cout << "test(int a);" << std::endl;
}

// read multiple value types
void test_two_args(int a, const char* b) {
	std::cout << "test(int a, const char* b);" << std::endl;
}

// value-type return value
int square(int a) {
	return a * a;
}

// C-style string tests
const char* get_const_c_string() {
	return "butts_const";
}

// C++-style string tests
std::string get_cpp_string() {
	return std::string("potato");
}

const std::string& get_const_ref_cpp_string() {
	static const std::string str("potato_const_ref");
	return str;
}

// should NOT work
std::string& get_ref_cpp_string() {
	static std::string str("potato_ref");
	return str;
}

// should NOT work
std::string* get_ptr_cpp_string() {
	static std::string str("potato_ptr");
	return &str;
}

class DogPrimitive {
public:
	DogPrimitive(const char* name) : mName(name) {
		sCount++;
	}
	virtual ~DogPrimitive() {
		sCount--;
	}

	static int count() {
		return sCount;
	}

	inline const std::string& name() const {
		return mName;
	}

private:
	static int sCount;
	std::string mName;
};

int DogPrimitive::sCount = 0;

void test_primitives() {
	duk_context* ctx = duk_create_heap_default();

	dukglue_register_function(ctx, test_no_args, "test_no_args");
	test_eval(ctx, "test_no_args();");
	duk_pop(ctx);
	//test_eval_expect_error(ctx, "test_no_args(42);");

	dukglue_register_function(ctx, test_one_arg, "test_one_arg");
	test_eval(ctx, "test_one_arg(7);");
	duk_pop(ctx);
	test_eval_expect_error(ctx, "test_one_arg('butts');");

	dukglue_register_function(ctx, test_two_args, "test_two_args");
	test_eval(ctx, "test_two_args(2, 'butts');");
	duk_pop(ctx);
	//test_eval_expect_error(ctx, "test_two_args(2, 'butts', 4);");
	test_eval_expect_error(ctx, "test_two_args('butts', 2);");
	test_eval_expect_error(ctx, "test_two_args('butts');");
	test_eval_expect_error(ctx, "test_two_args(2);");
	test_eval_expect_error(ctx, "test_two_args();");

	dukglue_register_function(ctx, square, "square");
	test_eval_expect(ctx, "square(4)", 16);
	test_eval_expect_error(ctx, "square('potato')");

	dukglue_register_function(ctx, get_const_c_string, "get_const_c_string");
	test_eval_expect(ctx, "get_const_c_string()", "butts_const");

	dukglue_register_function(ctx, get_cpp_string, "get_cpp_string");
	test_eval_expect(ctx, "get_cpp_string()", "potato");

	dukglue_register_function(ctx, get_const_ref_cpp_string, "get_const_ref_cpp_string");
	test_eval_expect(ctx, "get_const_ref_cpp_string()", "potato_const_ref");

	// this shouldn't compile and give a sane error message ("Value types can only be returned as const references.")
	//dukglue_register_function(ctx, get_ref_cpp_string, "get_ref_cpp_string");

	// this shouldn't compile and give a sane error message ("Cannot return pointer to value type.")
	//dukglue_register_function(ctx, get_ptr_cpp_string, "get_ptr_cpp_string");

	// std::vector for primitive types
	{
		std::vector<int> nums = { 1, 2, 3 };
		dukglue_push(ctx, nums);

		nums.clear();
		dukglue_read(ctx, -1, &nums);
		duk_pop(ctx);

		test_assert(nums.size() == 3);
		test_assert(nums.at(0) == 1);
		test_assert(nums.at(1) == 2);
		test_assert(nums.at(2) == 3);
	}

	// std::shared_ptr
	{
		test_assert(DogPrimitive::count() == 0);

		auto dog = std::make_shared<DogPrimitive>("Archie");
		test_assert(DogPrimitive::count() == 1);

		// can we push it?
		dukglue_push(ctx, dog);
		test_assert(DogPrimitive::count() == 1);

		// save it somewhere - does the shared_ptr persist? (i.e. deleter not called)
		duk_put_global_string(ctx, "testDog");
		test_assert(DogPrimitive::count() == 1);

		dog.reset();
		test_assert(DogPrimitive::count() == 1);

		// can we read it?
		duk_get_global_string(ctx, "testDog");
		dukglue_read< std::shared_ptr<DogPrimitive> >(ctx, -1, &dog);
		duk_pop(ctx);

		test_assert(dog->name() == "Archie");
		test_assert(DogPrimitive::count() == 1);
		dog.reset();

		// remove it completely (should trigger shared_ptr deleter after GC)
		duk_push_undefined(ctx);
		duk_put_global_string(ctx, "testDog");

		// intentionally called twice to make sure objects with finalizers are collected (see duk_gc docs)
		duk_gc(ctx, 0);
		duk_gc(ctx, 0);

		test_assert(DogPrimitive::count() == 0);
	}

	test_assert(duk_get_top(ctx) == 0);
	duk_destroy_heap(ctx);

	std::cout << "Primitives tested OK" << std::endl;
}
