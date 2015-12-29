#pragma once

#include "detail_stack.h"

namespace dukglue
{
	namespace detail
	{
		template<class Cls, typename... Ts>
		duk_ret_t call_native_constructor(duk_context* ctx)
		{
			if (!duk_is_constructor_call(ctx))
				return DUK_RET_TYPE_ERROR;

			try {
				std::tuple<Ts...> constructor_args = dukglue::detail::get_stack_values<Ts...>(ctx);
				Cls* obj = new Cls(); // TODO

				duk_push_this(ctx);
				duk_push_pointer(ctx, obj);
				duk_put_prop_string(ctx, -2, "ptr");
				duk_pop(ctx); // pop this

				return 0;
			}
			catch (DukTypeErrorException&)
			{
				return DUK_RET_TYPE_ERROR;
			}
		}
	}
}