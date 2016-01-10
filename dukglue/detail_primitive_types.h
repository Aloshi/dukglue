#pragma once

#include "detail_types.h"

#include <stdint.h>

namespace dukglue {
	namespace types {
		#define DUKGLUE_SIMPLE_VALUE_TYPE(TYPE, DUK_IS_FUNC, DUK_GET_FUNC, DUK_PUSH_FUNC, PUSH_VALUE) \
		template<> \
		struct DukType<TYPE> { \
			typedef std::true_type IsValueType; \
			\
			template<typename FullT> \
			static TYPE read(duk_context* ctx, duk_idx_t arg_idx) { \
				if (DUK_IS_FUNC(ctx, arg_idx)) { \
					return static_cast<TYPE>(DUK_GET_FUNC(ctx, arg_idx)); \
				} else { \
					duk_error(ctx, DUK_RET_TYPE_ERROR, "Argument %d: expected " #TYPE, arg_idx); \
				} \
			} \
			\
			template<typename FullT> \
			static void push(duk_context* ctx, TYPE value) { \
				DUK_PUSH_FUNC(ctx, PUSH_VALUE); \
			} \
		};

		DUKGLUE_SIMPLE_VALUE_TYPE(uint8_t,  duk_is_number, duk_get_uint, duk_push_uint, value)
		DUKGLUE_SIMPLE_VALUE_TYPE(uint16_t, duk_is_number, duk_get_uint, duk_push_uint, value)
		DUKGLUE_SIMPLE_VALUE_TYPE(uint32_t, duk_is_number, duk_get_uint, duk_push_uint, value)
		DUKGLUE_SIMPLE_VALUE_TYPE(uint64_t, duk_is_number, duk_get_number, duk_push_number, value) // have to cast to double

		DUKGLUE_SIMPLE_VALUE_TYPE(int8_t,  duk_is_number, duk_get_int, duk_push_int, value)
		DUKGLUE_SIMPLE_VALUE_TYPE(int16_t, duk_is_number, duk_get_int, duk_push_int, value)
		DUKGLUE_SIMPLE_VALUE_TYPE(int32_t, duk_is_number, duk_get_int, duk_push_int, value)
		DUKGLUE_SIMPLE_VALUE_TYPE(int64_t, duk_is_number, duk_get_number, duk_push_number, value) // have to cast to double

		// signed char and unsigned char are surprisingly *both* different from char, at least in MSVC
		DUKGLUE_SIMPLE_VALUE_TYPE(char, duk_is_number, duk_get_int, duk_push_int, value)

		DUKGLUE_SIMPLE_VALUE_TYPE(float, duk_is_number, duk_get_number, duk_push_number, value)
		DUKGLUE_SIMPLE_VALUE_TYPE(double, duk_is_number, duk_get_number, duk_push_number, value)

		DUKGLUE_SIMPLE_VALUE_TYPE(std::string, duk_is_string, duk_get_string, duk_push_string, value.c_str())

		// char* is a bit tricky because its "bare type" should still be char*
		template<>
		struct Bare<char*> {
			typedef char* type;
		};
		template<>
		struct Bare<const char*> {
			typedef char* type;
		};

		// the storage type should also be const char* - if we don't do this, it will end up as just "char"
		template<>
		struct ArgStorage<char*> {
			typedef const char* type;
		};

		template<>
		struct DukType<char*> {
			typedef std::true_type IsValueType;

			template<typename FullT>
			static const char* read(duk_context* ctx, duk_idx_t arg_idx) {
				if (duk_is_string(ctx, arg_idx)) {
					return duk_get_string(ctx, arg_idx);
				} else {
					duk_error(ctx, DUK_RET_TYPE_ERROR, "Argument %d: expected string", arg_idx);
				}
			}

			template<typename FullT>
			static void push(duk_context* ctx, const char* value) {
				duk_push_string(ctx, value);
			}
		};
	}
}