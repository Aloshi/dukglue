#include "test_assert.h"
#include <dukglue/dukglue.h>

#include <iostream>

const char* getNonsense() {
	return "desu";
}

class A {
public:
	int getMeaningOfLife() {
		return 42;
	}
};

class B {
public:
	const char* getMeaningOfLife() {
		return "whatever makes you happy";
	}
};

void test_multiple_contexts() {
	duk_context* ctx1 = duk_create_heap_default();
	duk_context* ctx2 = duk_create_heap_default();

	dukglue_register_function(ctx1, getNonsense, "getNonsense");
	test_eval_expect(ctx1, "getNonsense()", "desu");
	test_eval_expect_error(ctx2, "getNonsense()");

	dukglue_register_constructor<A>(ctx1, "TestClass");
	dukglue_register_delete<A>(ctx1);
	dukglue_register_method(ctx1, &A::getMeaningOfLife, "getMeaningOfLife");

	test_eval(ctx1, "var test = new TestClass();");
	duk_pop(ctx1);
	test_eval_expect_error(ctx2, "var test = new TestClass();");

	dukglue_register_constructor<B>(ctx2, "TestClass");
	dukglue_register_delete<B>(ctx2);
	dukglue_register_method(ctx2, &B::getMeaningOfLife, "getMeaningOfLife");

	test_eval(ctx2, "var test = new TestClass();");
	duk_pop(ctx2);

	test_eval_expect(ctx1, "test.getMeaningOfLife()", 42);
	test_eval_expect(ctx2, "test.getMeaningOfLife()", "whatever makes you happy");

	test_eval(ctx1, "test.delete()");
	duk_pop(ctx1);
	test_eval(ctx2, "test.delete()");
	duk_pop(ctx2);

	test_eval_expect_error(ctx1, "test.getMeaningOfLife()");
	test_eval_expect_error(ctx2, "test.getMeaningOfLife()");

	test_assert(duk_get_top(ctx1) == 0);
	test_assert(duk_get_top(ctx2) == 0);
	duk_destroy_heap(ctx1);
	duk_destroy_heap(ctx2);

	std::cout << "Multiple contexts tested OK" << std::endl;
}
