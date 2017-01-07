#include "test_assert.h"
#include <dukglue/dukglue.h>

#include <iostream>

class MyClass {
public:
	MyClass() : mValue(0) {}

	int getValue() const {
		return mValue;
	}
	void setValue(int v) {
		mValue = v;
	}

	int getValueNonConst() {
		return mValue;
	}

private:
	int mValue;
};

void test_properties()
{
	duk_context* ctx = duk_create_heap_default();

	dukglue_register_constructor<MyClass>(ctx, "MyClass");

	// test const getter + no setter
	dukglue_register_property(ctx, &MyClass::getValue, nullptr, "value");

	test_eval(ctx, "test = new MyClass()");
	duk_pop(ctx);
	test_eval_expect(ctx, "test.value", 0);
	test_eval_expect_error(ctx, "test.value = 42");
	
	// test const getter + setter
	dukglue_register_property(ctx, &MyClass::getValue, &MyClass::setValue, "value");
	test_eval(ctx, "test.value = 42");
	duk_pop(ctx);
	test_eval_expect(ctx, "test.value", 42);
	test_eval_expect_error(ctx, "test.value = \"food\"");

	// test no getter + setter
	dukglue_register_property(ctx, nullptr, &MyClass::setValue, "value");
	test_eval(ctx, "test.value = 128");
	duk_pop(ctx);
	test_eval_expect_error(ctx, "test.value");

	// test non-const getter + setter
	dukglue_register_property(ctx, &MyClass::getValueNonConst, &MyClass::setValue, "value");
	test_eval(ctx, "test.value = 64");
	duk_pop(ctx);
	test_eval_expect(ctx, "test.value", 64);

	// test non-const getter + no setter
	dukglue_register_property(ctx, &MyClass::getValueNonConst, nullptr, "value");
	test_eval_expect_error(ctx, "test.value = 256");
	test_eval_expect(ctx, "test.value", 64);

	test_assert(duk_get_top(ctx) == 0);
	duk_destroy_heap(ctx);

	std::cout << "Properties tested OK" << std::endl;
}