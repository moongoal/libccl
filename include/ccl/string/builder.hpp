/**
 * @file
 *
 * Basic string builder.
 */
#ifndef CCL_STRING_BUILDER_HPP
#define CCL_STRING_BUILDER_HPP

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
            using vector_type = vector<value_type, Allocator>;
            using size_type = typename vector_type::size_type;

            static constexpr unsigned default_int_buffer_size = 128;

        private:
            vector_type _data;

        public:
            explicit constexpr basic_string_builder(
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS,
                allocator_type * const allocator = nullptr
            ) : _data{alloc_flags, allocator}
            {}

            explicit constexpr basic_string_builder(
                const allocation_flags alloc_flags
            ) : basic_string_builder{nullptr, alloc_flags}
            {}

            explicit constexpr basic_string_builder(
                const string_type &s
            ) : _data{s, s.get_allocation_flags(), s.get_allocator()}
            {}

            template<size_type N>
            explicit constexpr basic_string_builder(
                const CharType (&raw)[N],
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS,
                allocator_type * const allocator = nullptr
            ) : _data{std::span{raw, N - 1}, alloc_flags, allocator}
            {}

            constexpr basic_string_builder(const basic_string_builder &other) : _data{other._data} {}

            constexpr basic_string_builder(basic_string_builder &&other) {
                swap(other);
            }

            constexpr auto swap(basic_string_builder &other) noexcept {
                _data.swap(other._data);
            }

            constexpr basic_string_builder& operator <<(const bool value) {
                _data.push_back(choose('1', '0', value));

                return *this;
            }

            constexpr basic_string_builder& operator <<(const short value) {
                value_type buff[default_int_buffer_size];
                char_traits::assign(buff[0], char_traits::nul());

                const size_type data_start = _data.size();
                const size_type written_chars = ::snprintf(buff, default_int_buffer_size, "%i", value);

                _data.resize(_data.size() + written_chars);
                char_traits::move(_data.data() + data_start, buff, written_chars);

                return *this;
            }

            constexpr basic_string_builder& operator <<(const long value) {
                value_type buff[default_int_buffer_size];
                char_traits::assign(buff[0], char_traits::nul());

                const size_type data_start = _data.size();
                const size_type written_chars = ::snprintf(buff, default_int_buffer_size, "%li", value);
                _data.resize(_data.size() + written_chars);
                char_traits::move(_data.data() + data_start, buff, written_chars);

                return *this;
            }

            constexpr basic_string_builder& operator <<(const unsigned long value) {
                value_type buff[default_int_buffer_size];
                char_traits::assign(buff[0], char_traits::nul());

                const size_type data_start = _data.size();
                const size_type written_chars = ::snprintf(buff, default_int_buffer_size, "%lu", value);
                _data.resize(_data.size() + written_chars);
                char_traits::move(_data.data() + data_start, buff, written_chars);

                return *this;
            }

            constexpr basic_string_builder& operator <<(const long long value) {
                value_type buff[default_int_buffer_size];
                char_traits::assign(buff[0], char_traits::nul());

                const size_type data_start = _data.size();
                const size_type written_chars = ::snprintf(buff, default_int_buffer_size, "%lli", value);
                _data.resize(_data.size() + written_chars);
                char_traits::move(_data.data() + data_start, buff, written_chars);

                return *this;
            }

            constexpr basic_string_builder& operator <<(const unsigned long long value) {
                value_type buff[default_int_buffer_size];
                char_traits::assign(buff[0], char_traits::nul());

                const size_type data_start = _data.size();
                const size_type written_chars = ::snprintf(buff, default_int_buffer_size, "%llu", value);
                _data.resize(_data.size() + written_chars);

                return *this;
            }

            constexpr basic_string_builder& operator <<(const double value) {
                value_type buff[default_int_buffer_size];
                char_traits::assign(buff[0], char_traits::nul());

                const size_type data_start = _data.size();
                const size_type written_chars = min<int, int>(
                    ::snprintf(buff, default_int_buffer_size, "%lf", value),
                    default_int_buffer_size
                );

                _data.resize(_data.size() + written_chars);
                char_traits::move(_data.data() + data_start, buff, written_chars);

                return *this;
            }

            constexpr basic_string_builder& operator <<(const long double value) {
                value_type buff[default_int_buffer_size];
                char_traits::assign(buff[0], char_traits::nul());

                const size_type data_start = _data.size();
                const size_type written_chars = min<int, int>(
                    ::snprintf(buff, default_int_buffer_size, "%Lf", value),
                    default_int_buffer_size
                );

                _data.resize(_data.size() + written_chars);
                char_traits::move(_data.data() + data_start, buff, written_chars);

                return *this;
            }

            constexpr basic_string_builder& operator <<(const int value) {
                value_type buff[default_int_buffer_size];
                char_traits::assign(buff[0], char_traits::nul());

                const size_type data_start = _data.size();
                const size_type written_chars = ::snprintf(buff, default_int_buffer_size, "%d", value);

                _data.resize(_data.size() + written_chars);
                char_traits::move(_data.data() + data_start, buff, written_chars);

                return *this;
            }

            constexpr basic_string_builder& operator <<(const unsigned value) {
                value_type buff[default_int_buffer_size];
                char_traits::assign(buff[0], char_traits::nul());

                const size_type data_start = _data.size();
                const size_type written_chars = ::snprintf(buff, default_int_buffer_size, "%u", value);

                _data.resize(_data.size() + written_chars);
                char_traits::move(_data.data() + data_start, buff, written_chars);

                return *this;
            }

            constexpr basic_string_builder& operator <<(const string_type &value) {
                const auto dest_begin = _data.size();

                _data.resize(_data.size() + value.length());
                char_traits::copy(_data.data() + dest_begin, &value[0], value.length());

                return *this;
            }

            constexpr basic_string_builder& append(const CharType * const value, const size_type length) {
                const auto dest_begin = _data.size();

                _data.resize(_data.size() + length);
                char_traits::copy(_data.data() + dest_begin, value, length);

                return *this;
            }

            template<size_type N>
            constexpr basic_string_builder& operator <<(const CharType (&value)[N]) {
                const auto dest_begin = _data.size();

                _data.resize(_data.size() + N - 1);
                char_traits::copy(_data.data() + dest_begin, value, N - 1);

                return *this;
            }

            constexpr basic_string_builder& operator <<(const CharType c) {
                const auto dest_begin = _data.size();
                _data.resize(_data.size() + 1);
                char_traits::assign(*(_data.data() + dest_begin), c);

                return *this;
            }

            constexpr allocator_type* get_allocator() const noexcept { return _data.get_allocator(); }
            constexpr allocation_flags get_allocation_flags() const noexcept { return _data.get_allocation_flags(); }

            string_type to_string() const {
                if(_data.is_empty()) {
                    return string_type{_data.get_allocation_flags(), _data.get_allocator()};
                }

                return string_type{_data.data(), _data.size(), _data.get_allocation_flags(), _data.get_allocator()};
            }

            constexpr void reserve(const size_type length) {
                _data.reserve(length);
            }
    };
}

#endif // CCL_STRING_BUILDER_HPP
