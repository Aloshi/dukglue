#pragma once

#include "detail_class.h"
#include "detail_method.h"

template<class Cls, typename... Ts>
void dukglue_register_class(duk_context* ctx, const char* name)
{
	duk_c_function constructorFunc = dukglue::detail::call_native_constructor<Cls>;

	duk_push_c_function(ctx, constructorFunc, sizeof...(Ts));

	// put an empty object for the constructor.prototype
	duk_push_object(ctx);
	duk_put_prop_string(ctx, -2, "prototype");

	// set name = constructor
	duk_put_global_string(ctx, name);
}

template<class Cls, typename T, T Value, typename RetType, typename... Ts>
void dukglue_register_method(duk_context* ctx, const char* className, const char* name, RetType(Cls::*func)(Ts...))
{
	duk_c_function methodFunc = dukglue::detail::MethodInfoHolder<Cls, RetType, Ts...>::MethodActual<Value>::call_native_method;

	duk_get_global_string(ctx, className);
	duk_get_prop_string(ctx, -1, "prototype");

	duk_push_c_function(ctx, methodFunc, sizeof...(Ts));
	duk_put_prop_string(ctx, -2, name); // consumes func above

	duk_pop_2(ctx); // pop prototype + constructor func
}