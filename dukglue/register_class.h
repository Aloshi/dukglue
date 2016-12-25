#pragma once

#include "detail_class_proto.h"
#include "detail_constructor.h"
#include "detail_method.h"

// Set the constructor for the given type.
template<class Cls, typename... Ts>
void dukglue_register_constructor(duk_context* ctx, const char* name)
{
	duk_c_function constructor_func = dukglue::detail::call_native_constructor<Cls, Ts...>;

	duk_push_c_function(ctx, constructor_func, sizeof...(Ts));

	// set constructor_func.prototype
	dukglue::detail::ProtoManager::push_prototype<Cls>(ctx);
	duk_put_prop_string(ctx, -2, "prototype");

	// set name = constructor_func
	duk_put_global_string(ctx, name);
}

/*
extern "C" duk_ret_t dummy_constructor(duk_context *ctx);

template<class Cls, typename... Ts>
void dukglue_register_prototype(duk_context* ctx, const char* name)
{
	duk_push_c_function(ctx, dummy_constructor, 0);
	dukglue::detail::ProtoManager::push_prototype<Cls>(ctx);
	duk_put_prop_string(ctx, -2, "prototype");
	duk_put_global_string(ctx, name);
}
*/

template<class Cls>
void dukglue_put_local_native_object(duk_context* ctx, Cls *object)
{
    dukglue::detail::ProtoManager::push_prototype<Cls>(ctx);
    duk_push_pointer(ctx, object);
    duk_put_prop_string(ctx, -2, "\xFF" "obj_ptr");
}

template<class Cls>
void dukglue_put_global_native_object(duk_context* ctx, Cls *object, const char* name)
{
    dukglue_put_local_native_object(ctx,object);
    duk_put_global_string(ctx, name);
}

template<class Base, class Derived>
void dukglue_set_base_class(duk_context* ctx)
{
	static_assert(!std::is_pointer<Base>::value && !std::is_pointer<Derived>::value
		&& !std::is_const<Base>::value && !std::is_const<Derived>::value, "Use bare class names.");
	static_assert(std::is_base_of<Base, Derived>::value, "Invalid class hierarchy!");

  using namespace dukglue::detail;

	// Derived.type_info->set_base(Base.type_info)
	ProtoManager::push_prototype<Derived>(ctx);
	duk_get_prop_string(ctx, -1, "\xFF" "type_info");
	TypeInfo* derived_type_info = static_cast<TypeInfo*>(duk_require_pointer(ctx, -1));
	duk_pop_2(ctx);

	ProtoManager::push_prototype<Base>(ctx);
	duk_get_prop_string(ctx, -1, "\xFF" "type_info");
	TypeInfo* base_type_info = static_cast<TypeInfo*>(duk_require_pointer(ctx, -1));
	duk_pop_2(ctx);

	derived_type_info->set_base(base_type_info);

	// also set up the prototype chain
	ProtoManager::push_prototype<Derived>(ctx);
	ProtoManager::push_prototype<Base>(ctx);
	duk_set_prototype(ctx, -2);
	duk_pop(ctx);
}

// methods
template<typename T, T Value, class Cls, typename RetType, typename... Ts>
void dukglue_register_method_compiletime(duk_context* ctx, RetType(Cls::*method)(Ts...), const char* name)
{
	static_assert(std::is_same<T, RetType(Cls::*)(Ts...)>::value, "Mismatching method types.");
	dukglue_register_method_compiletime<false, T, Value, Cls, RetType, Ts...>(ctx, name);
}

template<typename T, T Value, class Cls, typename RetType, typename... Ts>
void dukglue_register_method_compiletime(duk_context* ctx, RetType(Cls::*method)(Ts...) const, const char* name)
{
	static_assert(std::is_same<T, RetType(Cls::*)(Ts...) const>::value, "Mismatching method types.");
	dukglue_register_method_compiletime<true, T, Value, Cls, RetType, Ts...>(ctx, name);
}

template<bool isConst, typename T, T Value, class Cls, typename RetType, typename... Ts>
void dukglue_register_method_compiletime(duk_context* ctx, const char* name)
{
  using namespace dukglue::detail;
	typedef MethodInfo<isConst, Cls, RetType, Ts...> MethodInfo;

	duk_c_function method_func = MethodInfo::template MethodCompiletime<Value>::call_native_method;

	ProtoManager::push_prototype<Cls>(ctx);

	duk_push_c_function(ctx, method_func, sizeof...(Ts));
	duk_put_prop_string(ctx, -2, name); // consumes func above

	duk_pop(ctx); // pop prototype
}

template<class Cls, typename RetType, typename... Ts>
void dukglue_register_method(duk_context* ctx, RetType(Cls::*method)(Ts...), const char* name)
{
	dukglue_register_method<false, Cls, RetType, Ts...>(ctx, method, name);
}

template<class Cls, typename RetType, typename... Ts>
void dukglue_register_method(duk_context* ctx, RetType(Cls::*method)(Ts...) const, const char* name)
{
	dukglue_register_method<true, Cls, RetType, Ts...>(ctx, method, name);
}

// I'm sorry this signature is so long, but I figured it was better than duplicating the method,
// once for const methods and once for non-const methods.
template<bool isConst, typename Cls, typename RetType, typename... Ts>
void dukglue_register_method(duk_context* ctx, typename std::conditional<isConst, RetType(Cls::*)(Ts...) const, RetType(Cls::*)(Ts...)>::type method, const char* name)
{
	using namespace dukglue::detail;
	typedef MethodInfo<isConst, Cls, RetType, Ts...> MethodInfo;

	duk_c_function method_func = MethodInfo::MethodRuntime::call_native_method;

	ProtoManager::push_prototype<Cls>(ctx);

	duk_push_c_function(ctx, method_func, sizeof...(Ts));

	duk_push_pointer(ctx, new typename MethodInfo::MethodHolder{ method });
	duk_put_prop_string(ctx, -2, "\xFF" "method_holder"); // consumes raw method pointer

	// make sure we free the method_holder when this function is removed
	duk_push_c_function(ctx, MethodInfo::MethodRuntime::finalize_method, 1);
	duk_set_finalizer(ctx, -2);

	duk_put_prop_string(ctx, -2, name); // consumes method function

	duk_pop(ctx); // pop prototype
}

inline void dukglue_invalidate_object(duk_context* ctx, void* obj_ptr)
{
	dukglue::detail::RefManager::find_and_invalidate_native_object(ctx, obj_ptr);
}

// register a deleter
template<typename Cls>
void dukglue_register_delete(duk_context* ctx)
{
	duk_c_function delete_func = dukglue::detail::call_native_deleter<Cls>;

	dukglue::detail::ProtoManager::push_prototype<Cls>(ctx);
	duk_push_c_function(ctx, delete_func, 0);
	duk_put_prop_string(ctx, -2, "delete");
}