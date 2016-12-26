#include "test_assert.h"
#include <dukglue/dukglue.h>

#include <iostream>

void test_dukvalue()
{
	duk_context* ctx = duk_create_heap_default();

	// public_util.h test
	// ------------------

	// test push
	dukglue_push<int>(ctx, 2);
	duk_require_number(ctx, -1);

	// test read
	{
		int num = 0;
		dukglue_read<int>(ctx, -1, &num);
		test_assert(num == 2);
		duk_pop(ctx);
	}

	// DukValue tests
	// --------------

	// int (read/push)
	{
		test_eval(ctx, "42;");
		DukValue test = DukValue::take_from_stack(ctx);
		test_assert(test.type() == DukValue::NUMBER);
		test_assert(test.as_int() == 42);

		test.push();
		test_assert(duk_require_int(ctx, -1) == 42);
		duk_pop(ctx);
	}

	// double (read/push)
	{
		test_eval(ctx, "42.42;");
		DukValue test = DukValue::take_from_stack(ctx);
		test_assert(test.type() == DukValue::NUMBER);
		test_assert(test.as_double() == 42.42);

		test.push();
		test_assert(duk_require_number(ctx, -1) == 42.42);
		duk_pop(ctx);
	}

	// test ref counting (this is a pretty weak test, but it's better than nothing...)
	{
		test_eval(ctx, "new Object();");
		DukValue test = DukValue::take_from_stack(ctx);

		{
			DukValue test2 = test;
			DukValue test3 = test2;
		}
	}

	// test calling a callable
	{
		test_eval(ctx, "function testFunc(a, b) { return a + b; }; testFunc;");
		DukValue func = DukValue::take_from_stack(ctx);

		{
			int result = dukglue_call<int>(ctx, func, 12, 16);
			test_assert(result == 12 + 16);
		}

		// get the result as a DukValue
		{
			DukValue result = dukglue_call<DukValue>(ctx, func, 4, 6);
			test_assert(result.as_int() == 4 + 6);
		}

		// ignore the result
		{
			dukglue_call<void>(ctx, func, 4, 3);
		}
	}

	// test calling a callable with no return function
	{
		test_eval(ctx, "function doNothing() {}; doNothing;");
		DukValue func = DukValue::take_from_stack(ctx);

		// call expecting nothing
		{
			dukglue_call<void>(ctx, func);
		}

		// put into variant (this is valid - result should be undefined, since there was no return value)
		{
			DukValue result = dukglue_call<DukValue>(ctx, func);
			test_assert(result.type() == DukValue::UNDEFINED);
		}

		// error when we expect something
		{
			//dukglue_call<int>(ctx, func);
		}
	}

	// test calling a method
	{
		test_eval(ctx, "var testObj = new Object(); testObj.thingDo = function(v) { return v*v; }; testObj;");
		DukValue testObj = DukValue::take_from_stack(ctx, -1);

		// result as int
		{
			int result = dukglue_call_method<int>(ctx, testObj, "thingDo", 4);
			test_assert(result == 4 * 4);
		}

		// result as DukValue
		{
			DukValue result = dukglue_call_method<DukValue>(ctx, testObj, "thingDo", 3);
			test_assert(result.type() == DukValue::NUMBER);
			test_assert(result.as_int() == 3 * 3);
		}

		// ignore result
		{
			dukglue_call_method<void>(ctx, testObj, "thingDo", 3);
		}
	}

	// test calling a method with no return value
	{
		test_eval(ctx, "var testObj = new Object(); testObj.thingDo = function() {}; testObj;");
		DukValue testObj = DukValue::take_from_stack(ctx, -1);

		// call expecting nothing
		{
			dukglue_call_method<void>(ctx, testObj, "thingDo");
		}

		// put into variant (this is valid - result should be undefined, since there was no return value)
		{
			DukValue result = dukglue_call_method<DukValue>(ctx, testObj, "thingDo");
			test_assert(result.type() == DukValue::UNDEFINED);
		}

		// error when we expect something
		{
			//int result = dukglue_call_method<int>(ctx, testObj, "thingDo");
		}
	}

	// test DukValue's operator==
	{
		test_eval(ctx, "var testObj1 = new Object(); var testObj2 = new Object();");
		duk_pop(ctx);  // don't need undefined

		test_eval(ctx, "testObj1;");
		DukValue testObj1 = DukValue::take_from_stack(ctx);

		test_eval(ctx, "testObj2;");
		DukValue testObj2 = DukValue::take_from_stack(ctx);

		test_assert(testObj1 != testObj2);

		test_eval(ctx, "testObj1;");
		DukValue testObj1_again = DukValue::take_from_stack(ctx);

		test_assert(testObj1 == testObj1_again);
		test_assert(testObj1_again != testObj2);

		DukValue testObj1_copy = testObj1;
		test_assert(testObj1_copy == testObj1);
		test_assert(testObj1_copy == testObj1_again);
		test_assert(testObj1_copy != testObj2);
	}

	test_assert(duk_get_top(ctx) == 0);
	duk_destroy_heap(ctx);

	std::cout << "DukValue tested OK" << std::endl;
}