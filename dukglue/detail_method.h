#pragma once

#include "detail_stack.h"

namespace dukglue
{
	namespace detail
	{
		template<class Cls, typename RetType, typename... Ts>
		struct MethodInfoHolder
		{
			typedef RetType(Cls::*MethodType)(Ts...);

			template<MethodType methodToCall>
			struct MethodActual
			{
				static duk_ret_t call_native_method(duk_context* ctx)
				{
					// get this.ptr
					duk_push_this(ctx);
					duk_get_prop_string(ctx, -1, "ptr");
					void* obj_void = duk_require_pointer(ctx, -1);
					if (obj_void == nullptr) {
						duk_error(ctx, DUK_RET_REFERENCE_ERROR, "Native object missing.");
						return DUK_RET_REFERENCE_ERROR;
					}

					duk_pop_2(ctx);

					// (should always be valid unless someone is intentionally messing with this.ptr...)
					Cls* obj = static_cast<Cls*>(obj_void);

					// read arguments and call function
					// get_stack_values may throw a DukTypeErrorException
					try {
						actually_call<RetType>(ctx, obj, dukglue::detail::get_stack_values<Ts...>(ctx));
						return std::is_void<RetType>::value ? 0 : 1;
					}
					catch (DukTypeErrorException&)
					{
						return DUK_RET_TYPE_ERROR;
					}
				}

				// this mess is to support functions with void return values
				template<typename RetType2>
				inline static void actually_call(duk_context* ctx, Cls* obj, std::tuple<Ts...>&& args)
				{
					RetType return_val = dukglue::detail::apply_method<Cls, RetType, Ts...>(methodToCall, obj, args);
					push_value<RetType>(ctx, std::move(return_val));
				}

				template<>
				inline static void actually_call<void>(duk_context* ctx, Cls* obj, std::tuple<Ts...>&& args)
				{
					dukglue::detail::apply_method(methodToCall, obj, args);
				}
			};
		};
	}
}
