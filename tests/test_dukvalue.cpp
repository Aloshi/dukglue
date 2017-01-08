#include "test_assert.h"
#include <dukglue/dukglue.h>

#include <iostream>

class NativeObject {
public:
	double multiply(double a, double b) {
		return a*b;
	}
};

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

	// serialization (undefined)
	{
		std::vector<char> buff;

		{
			duk_push_undefined(ctx);
			DukValue v = DukValue::take_from_stack(ctx);
			buff = v.serialize();
		}

		DukValue v = DukValue::deserialize(ctx, buff.data(), buff.size());
		test_assert(v.type() == DukValue::UNDEFINED);
	}

	// serialization (null)
	{
		std::vector<char> buff;

		{
			duk_push_null(ctx);
			DukValue v = DukValue::take_from_stack(ctx);
			buff = v.serialize();
		}

		DukValue v = DukValue::deserialize(ctx, buff.data(), buff.size());
		test_assert(v.type() == DukValue::NULLREF);
	}

	// serialization (boolean)
	{
		std::vector<char> buff;

		{
			duk_push_boolean(ctx, true);
			DukValue v = DukValue::take_from_stack(ctx);
			buff = v.serialize();
		}

		DukValue v = DukValue::deserialize(ctx, buff.data(), buff.size());
		test_assert(v.type() == DukValue::BOOLEAN);
		test_assert(v.as_bool() == true);
	}

	// serialization (number)
	{
		std::vector<char> buff;

		{
			duk_push_number(ctx, 13.37);
			DukValue v = DukValue::take_from_stack(ctx);
			buff = v.serialize();
		}

		DukValue v = DukValue::deserialize(ctx, buff.data(), buff.size());
		test_assert(v.type() == DukValue::NUMBER);
		test_assert(v.as_double() == 13.37);
	}

	// serialization (string)
	{
		std::vector<char> buff;

		{
			duk_push_string(ctx, "this is a test");
			DukValue v = DukValue::take_from_stack(ctx);
			buff = v.serialize();
		}

		DukValue v = DukValue::deserialize(ctx, buff.data(), buff.size());
		test_assert(v.type() == DukValue::STRING);
		test_assert(v.as_string() == "this is a test");
	}

	// serialization (basic object)
	{
		std::vector<char> buff;

		{
			duk_push_object(ctx);
			duk_push_number(ctx, 42);
			duk_put_prop_string(ctx, -2, "x");
			duk_push_number(ctx, 369);
			duk_put_prop_string(ctx, -2, "y");
			DukValue v = DukValue::take_from_stack(ctx);
			buff = v.serialize();
		}

		DukValue v = DukValue::deserialize(ctx, buff.data(), buff.size());
		test_assert(v.type() == DukValue::OBJECT);
		v.push();

		duk_get_prop_string(ctx, -1, "x");
		test_assert(duk_require_number(ctx, -1) == 42);

		duk_get_prop_string(ctx, -2, "y");
		test_assert(duk_require_number(ctx, -1) == 369);

		duk_pop_3(ctx);
	}

	// std::vector<DukValue>
	{
		std::vector<DukValue> arr = dukglue_peval< std::vector<DukValue> >(ctx, "[1, true, 2.34, null, 'asdf']");
		test_assert(arr.size() == 5);
		test_assert(arr.at(0).type() == DukValue::NUMBER);
		test_assert(arr.at(0).as_int() == 1);
		test_assert(arr.at(1).type() == DukValue::BOOLEAN);
		test_assert(arr.at(1).as_bool() == true);
		test_assert(arr.at(2).type() == DukValue::NUMBER);
		test_assert(arr.at(2).as_double() == 2.34);
		test_assert(arr.at(3).type() == DukValue::NULLREF);
		test_assert(arr.at(4).type() == DukValue::STRING);
		test_assert(arr.at(4).as_string() == "asdf");
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
			int result = dukglue_pcall<int>(ctx, func, 12, 16);
			test_assert(result == 12 + 16);
		}

		// get the result as a DukValue
		{
			DukValue result = dukglue_pcall<DukValue>(ctx, func, 4, 6);
			test_assert(result.as_int() == 4 + 6);
		}

		// ignore the result
		{
			dukglue_pcall<void>(ctx, func, 4, 3);
		}
	}

	// test calling a callable with no return function
	{
		test_eval(ctx, "function doNothing() {}; doNothing;");
		DukValue func = DukValue::take_from_stack(ctx);

		// call expecting nothing
		{
			dukglue_pcall<void>(ctx, func);
		}

		// put into variant (this is valid - result should be undefined, since there was no return value)
		{
			DukValue result = dukglue_pcall<DukValue>(ctx, func);
			test_assert(result.type() == DukValue::UNDEFINED);
		}

		// error when we expect something
		{
			try {
				dukglue_pcall<int>(ctx, func);
				test_assert(false);
			} catch (DukException&) {
				// ok
			}
		}
	}

	// test calling a method
	{
		test_eval(ctx, "var testObj = new Object(); testObj.thingDo = function(v) { return v*v; }; testObj;");
		DukValue testObj = DukValue::take_from_stack(ctx, -1);

		// result as int
		{
			int result = dukglue_pcall_method<int>(ctx, testObj, "thingDo", 4);
			test_assert(result == 4 * 4);
		}

		// result as DukValue
		{
			DukValue result = dukglue_pcall_method<DukValue>(ctx, testObj, "thingDo", 3);
			test_assert(result.type() == DukValue::NUMBER);
			test_assert(result.as_int() == 3 * 3);
		}

		// ignore result
		{
			dukglue_pcall_method<void>(ctx, testObj, "thingDo", 3);
		}
	}

	// test calling a method with no return value
	{
		test_eval(ctx, "var testObj = new Object(); testObj.thingDo = function() {}; testObj;");
		DukValue testObj = DukValue::take_from_stack(ctx, -1);

		// call expecting nothing
		{
			dukglue_pcall_method<void>(ctx, testObj, "thingDo");
		}

		// put into variant (this is valid - result should be undefined, since there was no return value)
		{
			DukValue result = dukglue_pcall_method<DukValue>(ctx, testObj, "thingDo");
			test_assert(result.type() == DukValue::UNDEFINED);
		}

		// error when we expect something
		{
			try {
				int result = dukglue_pcall_method<int>(ctx, testObj, "thingDo");
				test_assert(false);
			} catch (DukException&) {
				// ok
			}
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

	// test pcall_method with native object
	{
		NativeObject* obj = new NativeObject();
		dukglue_register_global(ctx, obj, "obj");
		
		dukglue_register_method(ctx, &NativeObject::multiply, "multiplyNative");

		// create a method named testMethod on native object obj
		test_eval(ctx, "obj.testMethod = function(a, b) { this.lastA = a; this.lastB = b; return a*b; }");
		duk_pop(ctx);  // pop the undefined

		// call it, check the result
		{
			int result = dukglue_pcall_method<int>(ctx, obj, "testMethod", 4, 3);
			test_assert(result == 4 * 3);
		}

		{
			// call it, ignore the result (still has side effects)
			dukglue_pcall_method<void>(ctx, obj, "testMethod", 1, 2);

			// verify side effects went through (also checks that argument order is right)
			test_eval(ctx, "obj.lastA;");
			test_eval(ctx, "obj.lastB;");
			int lastA = duk_require_int(ctx, -2);
			int lastB = duk_require_int(ctx, -1);
			duk_pop_2(ctx);
			test_assert(lastA == 1);
			test_assert(lastB == 2);
		}

		// if we push the native object again, is the method still there?
		{
			// call method manually
			dukglue_push(ctx, obj);
			duk_get_prop_string(ctx, -1, "testMethod");
			duk_swap_top(ctx, -2);
			duk_push_int(ctx, 3);
			duk_push_int(ctx, 3);
			duk_pcall_method(ctx, 2);
			int stillWorks = duk_require_int(ctx, -1);
			duk_pop(ctx);
			test_assert(stillWorks == 3 * 3);
		}

		// test pcall_method with no arguments
		{
			dukglue_peval<void>(ctx, "obj.noArgsMethod = function() { return 42; }");
			
			int res = dukglue_pcall_method<int>(ctx, obj, "noArgsMethod");
			test_assert(res == 42);

			// test discarding result
			dukglue_pcall_method<void>(ctx, obj, "noArgsMethod");
		}

		// can we get a reference to the native object's script object as a DukValue?
		DukValue variantObj = dukglue_peval<DukValue>(ctx, "obj;");

		// DukValue ref works?
		{
			int res = dukglue_pcall_method<int>(ctx, variantObj, "testMethod", 4, 7);
			test_assert(res == 4 * 7);
		}

		dukglue_invalidate_object(ctx, obj);
		delete obj;
		obj = NULL;

		// make sure it invalidated correctly
		test_eval_expect_error(ctx, "obj.multiplyNative(4, 3);");

		// same for DukValue
		{
			try {
				int res = dukglue_pcall_method<int>(ctx, variantObj, "multiplyNative", 4, 3);
				test_assert(false);
			} catch (DukException&) {
				// ok
			}
		}
	}

	// test dukglue_peval with bad return result type
	{
		try {
			int test = dukglue_peval<int>(ctx, "'definitely not a number';");
			test_assert(false);
		} catch (DukException&) {
			// ok
		}
	}

	test_assert(duk_get_top(ctx) == 0);
	duk_destroy_heap(ctx);

	std::cout << "DukValue tested OK" << std::endl;
}