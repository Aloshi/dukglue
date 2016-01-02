#pragma once

#include "duktape/duktape.h"

#include <unordered_map>

namespace dukglue
{
	namespace detail
	{
		// This class handles keeping a map of void* -> script object.
		// It also prevents script objects from being GC'd until someone
		// explicitly frees the underlying native object.

		// Implemented by keeping an array of script objects in the heap stash.
		// An std::unordered_map maps pointer -> array index.
		// Thanks to std::unordered_map, lookup time is O(1) on average.

		// Using std::unordered_map has some memory overhead (~32 bytes per object),
		// which could be removed by using a different data structure:

		// 1. Use no data structure. Blindly scan through the reference registry,
		//    checking \xFFobj_ptr on every object until you find yours.
		//    Performance when returning native objects from functions when a lot
		//    of native objects are registered will suffer.

		// 2. Implement a self-balancing binary tree on top of a Duktape array
		//    for the registry. Still fast - O(log(N)) - and no memory overhead.

		struct RefManager
		{
		public:

			// Find the script object corresponding to obj_ptr and push it.
			// Returns true if successful, false if obj_ptr has not been registered.
			// Stack: ... -> ...              (if object has been registered before)
			//        ... -> ... [object]     (if object has not been registered)
			static bool find_and_push_native_object(duk_context* ctx, void* obj_ptr)
			{
				RefMap* ref_map = get_ref_map(ctx);

				const auto it = ref_map->find(obj_ptr);

				if (it == ref_map->end()) {
					return false;
				} else {
					push_ref_array(ctx);
					duk_get_prop_index(ctx, -1, it->second);
					duk_remove(ctx, -2);
					return true;
				}
			}

			// TODO make this use a free list

			// Takes a script object and adds it to the registry, associating
			// it with obj_ptr. unregistered_object is not modified.
			// If obj_ptr has already been registered with another object,
			// the old registry entry will be overidden.
			// Does nothing if obj_ptr is NULL.
			// Stack: ... [object]  ->  ... [object]
			static void register_native_object(duk_context* ctx, void* obj_ptr)
			{
				if (obj_ptr == NULL)
					return;

				RefMap* ref_map = get_ref_map(ctx);

				push_ref_array(ctx);

				const duk_uarridx_t ref_idx = duk_get_length(ctx, -1);
				ref_map->insert_or_assign(obj_ptr, ref_idx);

				duk_dup(ctx, -2);  // put object on top

				// ... [object] [ref_array] [object]
				duk_put_prop_index(ctx, -2, ref_idx);

				duk_pop(ctx);  // pop ref_array
			}

			// Remove the object associated with obj_ptr from the registry
			// and invalidate the object's internal native pointer (by setting it to undefined).
			// Does nothing if obj_ptr if object was never registered or obj_ptr is NULL.
			// Does not affect the stack.
			static void find_and_invalidate_native_object(duk_context* ctx, void* obj_ptr)
			{
				if (obj_ptr == NULL)
					return;

				RefMap* ref_map = get_ref_map(ctx);
				auto it = ref_map->find(obj_ptr);
				if (it == ref_map->end())  // was never registered
					return;

				push_ref_array(ctx);
				duk_get_prop_index(ctx, -1, it->second);

				// invalidate internal pointer
				duk_push_undefined(ctx);
				duk_put_prop_string(ctx, -2, "\xFF" "obj_ptr");

				// remove from references array (by setting to null)
				duk_push_null(ctx);
				duk_put_prop_index(ctx, -2, it->second);

				duk_pop_2(ctx);  // pop ref_array and the old script object

				// also remove from map
				ref_map->erase(it);
			}

		private:
			typedef std::unordered_map<void*, duk_uarridx_t> RefMap;

			static RefMap* get_ref_map(duk_context* ctx)
			{
				static const char* DUKGLUE_REF_MAP = "dukglue_ref_map";
				static const char* PTR = "ptr";

				duk_push_heap_stash(ctx);

				if (!duk_has_prop_string(ctx, -1, DUKGLUE_REF_MAP)) {
					// doesn't exist yet, need to create it
					duk_push_object(ctx);

					duk_push_pointer(ctx, new RefMap());
					duk_put_prop_string(ctx, -2, PTR);

					duk_push_c_function(ctx, ref_map_finalizer, 1);
					duk_set_finalizer(ctx, -2);

					duk_put_prop_string(ctx, -2, DUKGLUE_REF_MAP);
				}

				duk_get_prop_string(ctx, -1, DUKGLUE_REF_MAP);
				duk_get_prop_string(ctx, -1, PTR);
				RefMap* map = static_cast<RefMap*>(duk_require_pointer(ctx, -1));
				duk_pop_3(ctx);

				return map;
			}

			static duk_ret_t ref_map_finalizer(duk_context* ctx)
			{
				duk_get_prop_string(ctx, 0, "ptr");
				RefMap* map = static_cast<RefMap*>(duk_require_pointer(ctx, -1));
				delete map;

				return 0;
			}

			static void push_ref_array(duk_context* ctx)
			{
				static const char* DUKGLUE_REF_ARRAY = "dukglue_ref_array";
				duk_push_heap_stash(ctx);

				if (!duk_has_prop_string(ctx, -1, DUKGLUE_REF_ARRAY)) {
					duk_push_array(ctx);
					duk_put_prop_string(ctx, -2, DUKGLUE_REF_ARRAY);
				}

				duk_get_prop_string(ctx, -1, DUKGLUE_REF_ARRAY);
				duk_remove(ctx, -2); // pop heap stash
			}
		};
	}
}