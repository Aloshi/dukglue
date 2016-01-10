#pragma once

#include "detail_stack.h"
#include "detail_traits.h"
#include "detail_typeinfo.h"

#include <map>

#include <assert.h>

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

			static void push_prototype(duk_context* ctx)
			{
				if (!find_and_push_prototype(ctx)) {
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
					// If you're memory paranoid: this duplicates the type_info pointer
					// once per registered class. If you don't care about freeing memory
					// during shutdown, you can probably comment out this part.
					duk_push_object(ctx);
					duk_push_pointer(ctx, info);
					duk_put_prop_string(ctx, -2, "\xFF" "type_info");
					duk_push_c_function(ctx, type_info_finalizer, 1);
					duk_set_finalizer(ctx, -2);
					duk_put_prop_string(ctx, -2, "\xFF" "type_info_finalizer");

					// register it in the stash
					register_prototype(ctx, info);
				}
			}

			static void make_script_object(duk_context* ctx, Cls* obj)
			{
				duk_push_object(ctx);

				duk_push_pointer(ctx, obj);
				duk_put_prop_string(ctx, -2, "\xFF" "obj_ptr");

				push_prototype(ctx);
				duk_set_prototype(ctx, -2);
			}

		private:
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
					// nope, we need to create it
					duk_push_array(ctx);
					duk_put_prop_string(ctx, -2, DUKGLUE_PROTOTYPES);
				}

				duk_get_prop_string(ctx, -1, DUKGLUE_PROTOTYPES);

				// remove the heap stash from the stack
				duk_remove(ctx, -2);
			}

			// Stack: ... [proto]  ->  ... [proto]
			static void register_prototype(duk_context* ctx, const TypeInfo* info) {
				// 1. We assume info is not in the prototype array already
				// 2. Duktape has no efficient "shift array indices" operation (at least publicly)
				// 3. This method doesn't need to be fast, it's only called during registration

				// Work from high to low in the prototypes array, shifting as we go,
				// until we find the spot for info.

				push_prototypes_array(ctx);
				duk_size_t i = duk_get_length(ctx, -1);
				while (i > 0) {
					duk_get_prop_index(ctx, -1, i - 1);

					duk_get_prop_string(ctx, -1, "\xFF" "type_info");
					const TypeInfo* chk_info = static_cast<TypeInfo*>(duk_require_pointer(ctx, -1));
					duk_pop(ctx);  // pop type_info

					if (*chk_info > *info) {
						duk_put_prop_index(ctx, -2, i);
						i--;
					} else {
						duk_pop(ctx);  // pop prototypes_array[i]
						break;
					}
				}

				std::cout << "Registering prototype for " << typeid(Cls).name() << " at " << i << std::endl;

				duk_dup(ctx, -2);  // copy proto to top
				duk_put_prop_index(ctx, -2, i);
				duk_pop(ctx);  // pop prototypes_array
			}

			static bool find_and_push_prototype(duk_context* ctx) {
				const TypeInfo search_info = TypeInfo(typeid(Cls));

				push_prototypes_array(ctx);

				// these are ints and not duk_size_t to deal with negative indices
				int min = 0;
				int max = duk_get_length(ctx, -1) - 1;

				while (min <= max) {
					int mid = (max - min) / 2 + min;

					duk_get_prop_index(ctx, -1, mid);

					duk_get_prop_string(ctx, -1, "\xFF" "type_info");
					TypeInfo* mid_info = static_cast<TypeInfo*>(duk_require_pointer(ctx, -1));
					duk_pop(ctx);  // pop type_info pointer

					if (*mid_info == search_info) {
						// found it
						duk_remove(ctx, -2);  // pop prototypes_array
						return true;
					}
					else if (*mid_info < search_info) {
						min = mid + 1;
					}
					else {
						max = mid - 1;
					}

					duk_pop(ctx);  // pop prototypes_array[mid]
				}

				duk_pop(ctx);  // pop prototypes_array
				return false;
			}
		};
	}
}