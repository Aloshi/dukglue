#pragma once

#include "detail_stack.h"
#include "detail_traits.h"

namespace dukglue {
  namespace detail {

    template<typename Cls, typename... Ts>
    static duk_ret_t call_native_constructor(duk_context* ctx)
    {
      if (!duk_is_constructor_call(ctx)) {
        duk_error(ctx, DUK_RET_TYPE_ERROR, "Constructor must be called with new T().");
        return DUK_RET_TYPE_ERROR;
      }

      // construct the new instance
      auto constructor_args = dukglue::detail::get_stack_values<Ts...>(ctx);
      Cls* obj = apply_constructor<Cls, Ts...>(std::move(constructor_args));

      duk_push_this(ctx);

      // make the new script object keep the pointer to the new object instance
      duk_push_pointer(ctx, obj);
      duk_put_prop_string(ctx, -2, "\xFF" "obj_ptr");

      // register it
      dukglue::detail::RefManager::register_native_object(ctx, obj);

      duk_pop(ctx); // pop this

      return 0;
    }

  }
}
