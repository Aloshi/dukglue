#pragma once

#include "duktape.h"

void test_eval(duk_context* ctx, const char* code);

void test_eval_expect(duk_context* ctx, const char* code, const char* expected);
void test_eval_expect(duk_context* ctx, const char* code, int expected);

void test_eval_expect_error(duk_context* ctx, const char* code);

void test_assert(bool value);
