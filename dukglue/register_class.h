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

template<class Base, class Derived>
void dukglue_set_base_class(duk_context* ctx)
{
	static_assert(!std::is_pointer<Base>::value && !std::is_pointer<Derived>::value
		&& !std::is_const<Base>::value && !std::is_const<Derived>::value, "Use bare class names.");
	static_assert(std::is_base_of<Base, Derived>::value, "Invalid class hierarchy!");

	typedef dukglue::detail::ClassInfo<Base> BaseClassInfo;
	typedef dukglue::detail::ClassInfo<Derived> DerivedClassInfo;
	typedef dukglue::detail::TypeInfo TypeInfo;

	// Derived.type_info->set_base(Base.type_info)
	DerivedClassInfo::push_prototype(ctx);
	duk_get_prop_string(ctx, -1, "\xFF" "type_info");
	TypeInfo* derived_type_info = static_cast<TypeInfo*>(duk_require_pointer(ctx, -1));
	duk_pop_2(ctx);

	BaseClassInfo::push_prototype(ctx);
	duk_get_prop_string(ctx, -1, "\xFF" "type_info");
	TypeInfo* base_type_info = static_cast<TypeInfo*>(duk_require_pointer(ctx, -1));
	duk_pop_2(ctx);

	derived_type_info->set_base(base_type_info);

	// also set up the prototype chain
	DerivedClassInfo::push_prototype(ctx);
	BaseClassInfo::push_prototype(ctx);
	duk_set_prototype(ctx, -2);
	duk_pop(ctx);
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

	duk_c_function method_func = MethodInfo::MethodRuntime::call_native_method;

	ClassInfo::push_prototype(ctx);
	
	duk_push_c_function(ctx, method_func, sizeof...(Ts));

	duk_push_pointer(ctx, new MethodInfo::MethodHolder{method});
	duk_put_prop_string(ctx, -2, "\xFF" "method_holder"); // consumes raw method pointer

	// make sure we delete the method_holder when this function is removed
	duk_push_c_function(ctx, MethodInfo::MethodRuntime::finalize_method, 1);
	duk_set_finalizer(ctx, -2);

	duk_put_prop_string(ctx, -2, name); // consumes method function

	duk_pop(ctx); // pop prototype
}