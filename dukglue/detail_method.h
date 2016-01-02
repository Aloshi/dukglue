#pragma once

#include "detail_stack.h"

namespace dukglue
{
	namespace detail
	{
		template<class Cls, typename RetType, typename... Ts>
		struct MethodInfo
		{
			typedef RetType(Cls::*MethodType)(Ts...);

			// The size of a method pointer is not guaranteed to be the same size as a function pointer.
			// This means we can't just use duk_push_pointer(ctx, &MyClass::method) to store the method at run time.
			// To get around this, we wrap the method pointer in a MethodHolder (on the heap), and push a pointer to
			// that. The MethodHolder is cleaned up by the finalizer.
			struct MethodHolder
			{
				MethodType method;
			};

			template<MethodType methodToCall>
			struct MethodCompiletime
			{
				static duk_ret_t call_native_method(duk_context* ctx)
				{
					// get this.obj_ptr
					duk_push_this(ctx);
					duk_get_prop_string(ctx, -1, "\xFF" "obj_ptr");
					void* obj_void = duk_require_pointer(ctx, -1);
					if (obj_void == nullptr) {
						duk_error(ctx, DUK_RET_REFERENCE_ERROR, "Native object missing.");
						return DUK_RET_REFERENCE_ERROR;
					}

					duk_pop_2(ctx);

					// (should always be valid unless someone is intentionally messing with this.obj_ptr...)
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


			struct MethodRuntime
			{
				static duk_ret_t finalize_method(duk_context* ctx)
				{
					// clean up the MethodHolder reference
					duk_get_prop_string(ctx, 0, "\xFF" "method_holder");

					void* method_holder_void = duk_require_pointer(ctx, -1);
					MethodHolder* method_holder = static_cast<MethodHolder*>(method_holder_void);
					delete method_holder;

					return 0;
				}

				static duk_ret_t call_native_method(duk_context* ctx)
				{
					// get this.obj_ptr
					duk_push_this(ctx);
					duk_get_prop_string(ctx, -1, "\xFF" "obj_ptr");
					void* obj_void = duk_get_pointer(ctx, -1);
					if (obj_void == nullptr) {
						duk_error(ctx, DUK_RET_REFERENCE_ERROR, "Invalid native object for 'this'");
						return DUK_RET_REFERENCE_ERROR;
					}

					duk_pop_2(ctx); // pop this.obj_ptr and this

					// get current_function.method_info
					duk_push_current_function(ctx);
					duk_get_prop_string(ctx, -1, "\xFF" "method_holder");
					void* method_holder_void = duk_require_pointer(ctx, -1);
					if (method_holder_void == nullptr) {
						duk_error(ctx, DUK_RET_TYPE_ERROR, "Method pointer missing?!");
						return DUK_RET_TYPE_ERROR;
					}


					// (should always be valid unless someone is intentionally messing with this.obj_ptr...)
					Cls* obj = static_cast<Cls*>(obj_void);
					MethodHolder* method_holder = static_cast<MethodHolder*>(method_holder_void);

					// read arguments and call function
					// get_stack_values may throw a DukTypeErrorException
					try {
						actually_call<RetType>(ctx, method_holder->method, obj, dukglue::detail::get_stack_values<Ts...>(ctx));
						return std::is_void<RetType>::value ? 0 : 1;
					}
					catch (DukTypeErrorException&)
					{
						return DUK_RET_TYPE_ERROR;
					}
				}

				// this mess is to support functions with void return values
				template<typename RetType2>
				inline static void actually_call(duk_context* ctx, MethodType method, Cls* obj, std::tuple<Ts...>&& args)
				{
					RetType return_val = dukglue::detail::apply_method<Cls, RetType, Ts...>(method, obj, args);
					push_value<RetType>(ctx, std::move(return_val));
				}

				template<>
				inline static void actually_call<void>(duk_context* ctx, MethodType method, Cls* obj, std::tuple<Ts...>&& args)
				{
					dukglue::detail::apply_method(method, obj, args);
				}
			};
		};
	}
}