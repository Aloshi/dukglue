#include "test_assert.h"

#include "duktape.h"

#include <iostream>
#include <assert.h>

void test_eval(duk_context* ctx, const char* code)
{
	if (duk_peval_string(ctx, code) != 0) {
		duk_get_prop_string(ctx, -1, "stack");
		std::cerr << "Error running '" << code << "':" << std::endl;
		std::cerr << duk_safe_to_string(ctx, -1) << std::endl;
		duk_pop(ctx);

		test_assert(false);
	}
}

void test_eval_expect(duk_context* ctx, const char* code, const char* expected) {
	test_eval(ctx, code);
	const char* result = duk_require_string(ctx, -1);
	test_assert(strcmp(result, expected) == 0);
	duk_pop(ctx);
}

void test_eval_expect(duk_context* ctx, const char* code, int expected) {
	test_eval(ctx, code);
	int result = duk_require_int(ctx, -1);
	test_assert(result == expected);
	duk_pop(ctx);
}

void test_eval_expect_error(duk_context* ctx, const char* code) {
	if (duk_peval_string(ctx, code) == 0) {
		std::cerr << "Expected error running '" << code << "', but ran fine" << std::endl;
		test_assert(false);
	}

	duk_pop(ctx);  // ignore Error object
}

void test_assert(bool value) {
	assert(value);
}


void test_framework() {
	duk_context* ctx = duk_create_heap_default();

	test_eval(ctx, "var test = 2;");
	test_eval_expect(ctx, "test", 2);
	test_eval(ctx, "var str = 'Butts';");
	test_eval_expect(ctx, "str", "Butts");
	test_eval_expect_error(ctx, "#($*)@*(");
	test_assert(true);

	duk_destroy_heap(ctx);

	std::cout << "Framework OK" << std::endl;
}
