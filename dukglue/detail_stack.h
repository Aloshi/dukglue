#pragma once

#include <exception>
#include <string>

#include "detail_traits.h"

#include "duktape/duktape.h"

namespace dukglue
{
	namespace detail
	{
		// Thrown when a read_value<T>() call encounters a type mismatch.
		class DukTypeErrorException : public std::exception {};

		// Used to read a value from ctx at stack position arg_idx.
		// Will NOT try to coerce types.
		// If a value is not found on the stack, or the value is of the wrong type,
		// a DukTypeErrorException() will be thrown.
		template<typename T>
		T read_value(duk_context* ctx, duk_idx_t arg_idx);

		// Every read_value just changes around these four values, so I stuck it in a macro.
		#define DUK_READ_VALUE_MACRO(RET_TYPE, DUK_IS_FUNC, DUK_GET_FUNC, TYPE_NAME) \
		template<> \
		inline RET_TYPE read_value(duk_context* ctx, duk_idx_t arg_idx) \
		{ \
			if (DUK_IS_FUNC(ctx, arg_idx)) { \
				return RET_TYPE(DUK_GET_FUNC(ctx, arg_idx)); \
			} else { \
				duk_error(ctx, DUK_ERR_TYPE_ERROR, "Argument %d: expected %s", arg_idx, TYPE_NAME); \
				throw DukTypeErrorException(); \
			} \
		} \

		DUK_READ_VALUE_MACRO(int, duk_is_number, duk_get_number, "number")
		DUK_READ_VALUE_MACRO(float, duk_is_number, duk_get_number, "number")
		DUK_READ_VALUE_MACRO(double, duk_is_number, duk_get_number, "number")
		DUK_READ_VALUE_MACRO(std::string, duk_is_string, duk_get_string, "string")
		DUK_READ_VALUE_MACRO(bool, duk_is_boolean, 0 != duk_get_boolean, "boolean")  // "0 !=" to prevent a warning on MSVC

		template<typename T>
		void push_value(duk_context* ctx, const T& value);

		// Just calls duk_push_* for the given type.
		#define DUK_PUSH_VALUE_MACRO(TYPE, PUSH_FUNC, VALUE) \
		template<> \
		inline void push_value(duk_context* ctx, const TYPE& value) \
		{ \
			PUSH_FUNC(ctx, VALUE); \
		} \

		DUK_PUSH_VALUE_MACRO(int, duk_push_int, value);
		DUK_PUSH_VALUE_MACRO(float, duk_push_number, value);
		DUK_PUSH_VALUE_MACRO(double, duk_push_number, value);
		DUK_PUSH_VALUE_MACRO(std::string, duk_push_string, value.c_str());
		DUK_PUSH_VALUE_MACRO(bool, duk_push_boolean, value);

		// Call read_value for every Ts[i], for matching argument index Index[i].
		// The traits::index_tuple is used for type inference.
		// A concrete example:
		//   get_values<int, bool>(duktape_context)
		//     get_values_helper<{int, bool}, {0, 1}>(ctx, ignored)
		//       std::make_tuple<int, bool>(read_value<int>(ctx, 0), read_value<bool>(ctx, 1))
		template<typename... Ts, size_t ... Indexes>
		std::tuple<Ts...> get_stack_values_helper(duk_context* ctx, dukglue::detail::index_tuple <Indexes ...>)
		{
			return std::make_tuple(read_value<Ts>(ctx, Indexes)...);
		}

		// Returns an std::tuple of the values asked for in the template parameters.
		// Values will remain on the stack.
		// Values are indexed from the bottom of the stack up (0, 1, ...).
		// If a value does not exist or does not have the expected type, an error is thrown
		// on the Duktape stack (through duk_error(...)), and a DukTypeErrorException is thrown.
		template<typename... Ts>
		std::tuple<Ts...> get_stack_values(duk_context* ctx)
		{
			// We need the argument indices for read_value, and we need to be able
			// to unpack them as a template argument to match Ts.
			// So, we use traits::make_indexes<Ts...>, which returns a traits::index_tuple<0, 1, 2, ...> object.
			// We pass that into a helper function so we can put a name to that <0, 1, ...> template argument.
			auto indices = typename dukglue::detail::make_indexes<Ts...>::type();
			return get_stack_values_helper<Ts...>(ctx, indices);
		}
	}
}
