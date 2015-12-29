#pragma once

#include "detail_function.h"

template<typename T, T Value, typename RetType, typename... Ts>
void dukglue_register_function_compiletime(duk_context* ctx, const char* name, RetType(*)(Ts...))
{
	static_assert(std::is_same<T, RetType(Ts...)>::value,
		"Mismatching function pointer template parameter and function pointer argument types. "
		"Try: dukglue_register_function<decltype(func), func>(ctx, \"funcName\", func)");

	duk_c_function evalFunc = dukglue::detail::FuncInfoHolder<RetType, Ts...>::FuncActual<Value>::call_native_function;

	duk_push_c_function(ctx, evalFunc, sizeof...(Ts));
	duk_put_global_string(ctx, name);
}

template<typename RetType, typename... Ts>
void dukglue_register_function_runtime(duk_context* ctx, const char* name, RetType(*funcToCall)(Ts...))
{
	duk_c_function evalFunc = dukglue::detail::FuncInfoHolder<RetType, Ts...>::FuncRuntime::call_native_function;

	duk_push_c_function(ctx, evalFunc, sizeof...(Ts));

	duk_push_pointer(ctx, funcToCall);
	duk_put_prop_string(ctx, -2, "\xFF" "func_ptr");

	duk_put_global_string(ctx, name);
}