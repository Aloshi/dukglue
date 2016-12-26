#pragma once

// This file has some useful utility functions for users.
// Hopefully this saves you from wading through the implementation.

/**
 * @brief      Push a value onto the duktape stack.
 * @param      ctx    duktape context
 * @param[in]  val    value to push
 */
template <typename FullT>
void dukglue_push(duk_context* ctx, const FullT& val) {
	// ArgStorage has some static_asserts in it that validate value types,
	// so we typedef it to force ArgStorage<RetType> to compile and run the asserts
	typedef typename dukglue::types::ArgStorage<FullT>::type ValidateReturnType;

	using namespace dukglue::types;
	DukType<typename Bare<FullT>::type>::template push<FullT>(ctx, std::move(val));
}

template <typename T, typename... ArgTs>
void dukglue_push(duk_context* ctx, T arg, ArgTs... args)
{
	dukglue_push(ctx, arg);
	dukglue_push(ctx, args...);
}

inline void dukglue_push(duk_context* ctx)
{
	// no-op
}

template <typename RetT>
void dukglue_read(duk_context* ctx, duk_idx_t arg_idx, RetT* out)
{
	// ArgStorage has some static_asserts in it that validate value types,
	// so we typedef it to force ArgStorage<RetType> to compile and run the asserts
	typedef typename dukglue::types::ArgStorage<RetT>::type ValidateReturnType;

	using namespace dukglue::types;
	*out = DukType<typename Bare<RetT>::type>::template read<RetT>(ctx, arg_idx);
}

// TODO add pcall/pcall_method

template <typename RetT, typename ObjT, typename... ArgTs>
typename std::enable_if<std::is_void<RetT>::value, RetT>::type dukglue_call_method(duk_context* ctx, const ObjT& obj, const char* method_name, ArgTs... args)
{
	dukglue_push(ctx, obj);
	duk_get_prop_string(ctx, -1, method_name);

	if (duk_check_type(ctx, -1, DUK_TYPE_UNDEFINED))
		assert(false);  // method does not exist

	duk_swap_top(ctx, -2);
	dukglue_push(ctx, args...);
	duk_call_method(ctx, sizeof...(args));
	duk_pop(ctx);
}

template <typename RetT, typename ObjT, typename... ArgTs>
typename std::enable_if<!std::is_void<RetT>::value, RetT>::type dukglue_call_method(duk_context* ctx, const ObjT& obj, const char* method_name, ArgTs... args)
{
	dukglue_push(ctx, obj);
	duk_get_prop_string(ctx, -1, method_name);

	if (duk_check_type(ctx, -1, DUK_TYPE_UNDEFINED))
		assert(false);  // method does not exist

	duk_swap_top(ctx, -2);
	dukglue_push(ctx, args...);
	duk_call_method(ctx, sizeof...(args));

	RetT ret;
	dukglue_read(ctx, -1, &ret);
	duk_pop(ctx);
	return ret;
}

template <typename RetT, typename ObjT, typename... ArgTs>
typename std::enable_if<std::is_void<RetT>::value, RetT>::type dukglue_call(duk_context* ctx, const ObjT& obj, ArgTs... args)
{
	dukglue_push(ctx, obj);
	if (!duk_is_callable(ctx, -1))
		assert(false);  // obj is not callable

	dukglue_push(ctx, args...);
	duk_call(ctx, sizeof...(args));
	duk_pop(ctx);  // pop return value
}

template <typename RetT, typename ObjT, typename... ArgTs>
typename std::enable_if<!std::is_void<RetT>::value, RetT>::type dukglue_call(duk_context* ctx, const ObjT& obj, ArgTs... args)
{
	dukglue_push(ctx, obj);
	if (!duk_is_callable(ctx, -1))
		assert(false);  // obj is not callable

	dukglue_push(ctx, args...);
	duk_call(ctx, sizeof...(args));

	RetT ret;
	dukglue_read(ctx, -1, &ret);
	duk_pop(ctx);
	return ret;
}