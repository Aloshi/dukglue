#pragma once

#include "detail_stack.h"

namespace dukglue
{
	namespace detail
	{
		// This struct can be used to generate a Duktape C function that
		// pulls the argument values off the stack (with type checking),
		// calls the appropriate function with them, and puts the function's
		// return value (if any) onto the stack.
		template<typename RetType, typename... Ts>
		struct FuncInfoHolder
		{
			typedef RetType(*FuncType)(Ts...);

			template<FuncType funcToCall>
			struct FuncCompiletime
			{
				// The function to call is embedded into call_native_function at
				// compile-time through template magic.
				// Performance is so similar to run-time function calls that
				// this is not recommended due to the ugly syntax it requires.
				static duk_ret_t call_native_function(duk_context* ctx)
				{
					// get_stack_values may throw a DukTypeErrorException
					try {
						actually_call<RetType>(ctx, dukglue::detail::get_stack_values<Ts...>(ctx));
						return std::is_void<RetType>::value ? 0 : 1;
					}
					catch (DukTypeErrorException&) {
						return DUK_RET_TYPE_ERROR;
					}
				}

				// this mess is to support functions with void return values
				template<typename RetType2>
				inline static void actually_call(duk_context* ctx, std::tuple<Ts...>&& args)
				{
					RetType return_val = dukglue::detail::apply_fp(funcToCall, args);
					dukglue::detail::push_value<RetType>(ctx, std::move(return_val));
				}

				template<>
				inline static void actually_call<void>(duk_context* ctx, std::tuple<Ts...>&& args)
				{
					dukglue::detail::apply_fp(funcToCall, args);
				}
			};

			struct FuncRuntime
			{
				// Pull the address of the function to call from the
				// Duktape function object at run time.
				static duk_ret_t call_native_function(duk_context* ctx)
				{
					duk_push_current_function(ctx);
					duk_get_prop_string(ctx, -1, "\xFF" "func_ptr");
					void* fp_void = duk_require_pointer(ctx, -1);
					if (fp_void == NULL) {
						duk_error(ctx, DUK_RET_TYPE_ERROR, "what even");
						return DUK_RET_TYPE_ERROR;
					}

					duk_pop_2(ctx);

					RetType(*funcToCall)(Ts...) = static_cast<RetType(*)(Ts...)>(fp_void);

					// get_stack_values may throw a DukTypeErrorException
					try {
						actually_call<RetType>(ctx, funcToCall, dukglue::detail::get_stack_values<Ts...>(ctx));
						return std::is_void<RetType>::value ? 0 : 1;
					}
					catch (DukTypeErrorException&) {
						return DUK_RET_TYPE_ERROR;
					}
				}

				// this mess is to support functions with void return values
				template<typename RetType2>
				inline static void actually_call(duk_context* ctx, RetType(*funcToCall)(Ts...), std::tuple<Ts...>&& args)
				{
					RetType return_val = dukglue::detail::apply_fp(funcToCall, args);
					dukglue::detail::push_value<RetType>(ctx, std::move(return_val));
				}

				template<>
				inline static void actually_call<void>(duk_context* ctx, RetType(*funcToCall)(Ts...), std::tuple<Ts...>&& args)
				{
					dukglue::detail::apply_fp(funcToCall, args);
				}
			};
		};
	}
}