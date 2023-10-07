/**
 * @file
 *
 * Basic string builder.
 */
#ifndef CCL_STRING_BUILDER_HPP
#define CCL_STRING_BUILDER_HPP

#include <cstring>
#include <ccl/api.hpp>
#include <ccl/vector.hpp>
#include <ccl/concepts.hpp>
#include <ccl/string/basic-string.hpp>
#include <ccl/string/char-traits.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/util.hpp>

namespace ccl {
    template<
        typename CharType,
        char_traits_impl<CharType> CharTraits = char_traits<CharType>,
        typed_allocator<CharType> Allocator = allocator
    > class basic_string_builder {
        public:
            using value_type = CharType;
            using char_traits = CharTraits;
            using allocator_type = Allocator;
            using string_type = basic_string<value_type, char_traits, allocator_type>;

            static constexpr int default_int_radix = 10;
            static constexpr unsigned default_int_buffer_size = 128;

        private:
            int int_radix = default_int_radix;
            string_type _data;

        public:
            explicit constexpr basic_string_builder(
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : _data{allocator, alloc_flags}
            {}

            explicit constexpr basic_string_builder(
                const allocation_flags alloc_flags
            ) : basic_string_builder{nullptr, alloc_flags}
            {}

            constexpr basic_string_builder(const basic_string_builder &other) : _data{other._data} {}
            constexpr basic_string_builder(basic_string_builder &&other) : _data{std::move(other._data)} {}

            constexpr auto swap(basic_string_builder &other) noexcept {
                _data.swap(other._data);
            }

            constexpr void set_int_radix(const int radix) {
                int_radix = radix;
            }

            constexpr basic_string_builder& operator <<(const bool value) {
                _data.push_back(choose('1', '0', value));

                return *this;
            }

            constexpr basic_string_builder& operator <<(const short value) {
                value_type buff[default_int_buffer_size];

                buff[0] = char_traits::nul();

                #ifdef _WIN32
                    ::_itoa_s(value, buff, int_radix);
                #else // defined(_WIN32)
                    ::itoa(value, buff, int_radix);
                #endif // defined(_WIN32)

                _data.insert_range(_data.end(), buff);

                return *this;
            }
    };
}

#endif // CCL_STRING_BUILDER_HPP
