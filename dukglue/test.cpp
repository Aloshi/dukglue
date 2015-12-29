#include <iostream>
#include <ctime>

#include "dukglue.h"

#include "duktape/duktape.h"

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

// #define dukglue_register_func_cp(ctx, name, func) dukglue_register_function_compiletime<decltype(func), func>(ctx, name, func)

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

int main()
{
	duk_context* ctx = duk_create_heap_default();

	// ct
	dukglue_register_function_compiletime<decltype(testFunc), testFunc>(ctx, "testFunc1_ct", testFunc);
	dukglue_register_function_compiletime<decltype(testFunc), testFunc>(ctx, "testFunc2_ct", testFunc2);

	// rt
	dukglue_register_function_runtime(ctx, "testFunc1_rt", testFunc);
	dukglue_register_function_runtime(ctx, "testFunc2_rt", testFunc);

	timeTest("Compile-time generated function calls", ctx,
		"function main() {"
			"for (var i = 0; i < 5000; i++) {\n"
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
			"for (var i = 0; i < 5000; i++) {\n"
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
