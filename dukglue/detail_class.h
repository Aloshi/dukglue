#pragma once

#include "detail_stack.h"
#include "detail_traits.h"
#include "detail_typeinfo.h"

#include <map>

namespace dukglue
{
	namespace detail
	{
		template<class Cls>
		struct ClassInfo
		{
		public:
			template<typename... Ts>
			static duk_ret_t call_native_constructor(duk_context* ctx)
			{
				if (!duk_is_constructor_call(ctx)) {
					duk_error(ctx, DUK_RET_TYPE_ERROR, "Constructor must be called with new T().");
					return DUK_RET_TYPE_ERROR;
				}

				try {
					// construct the new instance
					std::tuple<Ts...> constructor_args = dukglue::detail::get_stack_values<Ts...>(ctx);
					Cls* obj = apply_constructor<Cls, Ts...>(std::move(constructor_args));

					duk_push_this(ctx);

					// make the new script object keep the pointer to the new object instance
					duk_push_pointer(ctx, obj);
					duk_put_prop_string(ctx, -2, "\xFF" "obj_ptr");

					duk_pop(ctx); // pop this

					return 0;
				}
				catch (DukTypeErrorException&)
				{
					return DUK_RET_TYPE_ERROR;
				}
			}

			static void push_prototype(duk_context* ctx)
			{
				// push (or create if it doesn't exist) the array
				// that holds all of our prototypes
				push_prototypes_array(ctx);

				// has this class's prototype been accessed before?
				if (s_heap_stash_idx_ == 0) {
					// nope, need to create our prototype object
					duk_push_object(ctx);

					// add reference to this class' info object so we can do type checking
					// when trying to pass this object into method calls
					typedef dukglue::detail::TypeInfo TypeInfo;
					TypeInfo* info = new TypeInfo(typeid(Cls));

					duk_push_pointer(ctx, info);
					duk_put_prop_string(ctx, -2, "\xFF" "type_info");

					// Clean up the TypeInfo object when this prototype is destroyed.
					// We can't put a finalizer directly on this prototype, because it
					// will be run whenever the wrapper for an object of this class is
					// destroyed; instead, we make a dummy object and put the finalizer
					// on that.
					// Warning if you're memory paranoid: this duplicates the type_info
					// pointer. If you don't care about freeing memory during shutdown,
					// you can probably comment out this part.
					duk_push_object(ctx);
					duk_push_pointer(ctx, info);
					duk_put_prop_string(ctx, -2, "\xFF" "type_info");
					duk_push_c_function(ctx, type_info_finalizer, 1);
					duk_set_finalizer(ctx, -2);
					duk_put_prop_string(ctx, -2, "\xFF" "type_info_finalizer");

					// register it in the stash and update our index
					s_heap_stash_idx_ = duk_get_length(ctx, -2);
					duk_put_prop_index(ctx, -2, s_heap_stash_idx_);
				}

				duk_get_prop_index(ctx, -1, s_heap_stash_idx_);

				// remove the prototype array from the stack
				duk_remove(ctx, -2);
			}

		private:
			static duk_size_t s_heap_stash_idx_;

			static duk_ret_t type_info_finalizer(duk_context* ctx)
			{
				duk_get_prop_string(ctx, 0, "\xFF" "type_info");
				dukglue::detail::TypeInfo* info = static_cast<dukglue::detail::TypeInfo*>(duk_require_pointer(ctx, -1));
				delete info;

				// set pointer to NULL in case this finalizer runs again
				duk_push_pointer(ctx, NULL);
				duk_put_prop_string(ctx, 0, "\xFF" "type_info");

				return 0;
			}

			// puts heap_stash["dukglue_prototypes"] on the stack,
			// or creates it if it doesn't exist
			static void push_prototypes_array(duk_context* ctx)
			{
				static const char* DUKGLUE_PROTOTYPES = "dukglue_prototypes";

				duk_push_heap_stash(ctx);

				// does the prototype array already exist?
				if (!duk_has_prop_string(ctx, -1, DUKGLUE_PROTOTYPES)) {
					// nope, we need to create it by doing:
					// heap_stash["dukglue_prototypes"] = [0];

					duk_push_array(ctx);

					// since s_heap_stash_idx_ is reserved to mean "not yet registered,"
					// we need to make sure dukglue_prototypes[0] never gets used
					// (dukglue_prototypes.length > 0). So: dukglue_prototypes[0] = null
					duk_push_null(ctx);
					duk_put_prop_index(ctx, -2, 0);

					duk_put_prop_string(ctx, -2, DUKGLUE_PROTOTYPES);
				}

				duk_get_prop_string(ctx, -1, DUKGLUE_PROTOTYPES);

				// remove the heap stash from the stack
				duk_remove(ctx, -2);
			}
		};

		// initialize static members
		template<class Cls>
		duk_size_t ClassInfo<Cls>::s_heap_stash_idx_ = 0; // 0 means no prototype has been created
	}
}