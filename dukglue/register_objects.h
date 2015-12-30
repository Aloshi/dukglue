#pragma once

#include "detail_class.h"
#include "detail_method.h"

// Set the constructor for the given type.
template<class Cls, typename... Ts>
void dukglue_register_constructor(duk_context* ctx, const char* name)
{
	typedef dukglue::detail::ClassInfo<Cls> ClassInfo;

	duk_c_function constructor_func = dukglue::detail::ClassInfo<Cls>::call_native_constructor<Ts...>;

	duk_push_c_function(ctx, constructor_func, sizeof...(Ts));

	// set constructor_func.prototype
	ClassInfo::push_prototype(ctx);
	duk_put_prop_string(ctx, -2, "prototype");

	// set name = constructor_func
	duk_put_global_string(ctx, name);
}

// methods
template<class Cls, typename T, T Value, typename RetType, typename... Ts>
void dukglue_register_method_compiletime(duk_context* ctx, RetType(Cls::*func)(Ts...), const char* name)
{
	typedef dukglue::detail::ClassInfo<Cls> ClassInfo;
	typedef dukglue::detail::MethodInfo<Cls, RetType, Ts...> MethodInfo;

	duk_c_function method_func = MethodInfo::MethodCompiletime<Value>::call_native_method;

	ClassInfo::push_prototype(ctx);

	duk_push_c_function(ctx, method_func, sizeof...(Ts));
	duk_put_prop_string(ctx, -2, name); // consumes func above

	duk_pop(ctx); // pop prototype
}

template<class Cls, typename RetType, typename... Ts>
void dukglue_register_method(duk_context* ctx, RetType(Cls::*method)(Ts...), const char* name)
{
	typedef dukglue::detail::ClassInfo<Cls> ClassInfo;
	typedef dukglue::detail::MethodInfo<Cls, RetType, Ts...> MethodInfo;

	duk_c_function method_func = dukglue::detail::MethodInfo<Cls, RetType, Ts...>::MethodRuntime::call_native_method;

	ClassInfo::push_prototype(ctx);
	
	duk_push_c_function(ctx, method_func, sizeof...(Ts));

	duk_push_pointer(ctx, new MethodInfo::MethodHolder{method});
	duk_put_prop_string(ctx, -2, "\xFF" "method_holder"); // consumes raw method pointer

	duk_put_prop_string(ctx, -2, name); // consumes method function

	duk_pop(ctx); // pop prototype
}