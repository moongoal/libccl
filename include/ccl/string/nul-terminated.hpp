/**
 * @file
 *
 * NUL-terminated string containers.
 *
 * These containers are intended for temporary nul-terminated string use.
 */
#ifndef CCL_STRING_NUL_TERMINATED_HPP
#define CCL_STRING_NUL_TERMINATED_HPP

#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/string/basic-string.hpp>
#include <ccl/string/ansi-string.hpp>
#include <ccl/util.hpp>
#include <ccl/internal/optional-allocator.hpp>

namespace ccl {
    constexpr count_t CCL_DEFAULT_NUL_TERMINATED_STRING_MAX_LENGTH = 256;

    template<
        typename CharType,
        count_t MaxLength = CCL_DEFAULT_NUL_TERMINATED_STRING_MAX_LENGTH
    > class nul_terminated_string {
        private:
            CharType data[MaxLength];
            count_t _length;

        public:
            constexpr count_t length() const noexcept { return _length; }

            nul_terminated_string(): _length{0} { data[0] = 0; }

            template<
                char_traits_impl<CharType> CharTraits,
                typed_allocator<CharType> Allocator
            > nul_terminated_string(const basic_string<CharType, CharTraits, Allocator> &str) : _length{str.length()} {
                str.to_nul_terminated(data);
            }

            constexpr const CharType *value() const noexcept { return data; }
    };

    template<
        typename CharType,
        char_traits_impl<CharType> CharTraits = char_traits<CharType>,
        typed_allocator<CharType> Allocator = allocator
    > class dynamic_nul_terminated_string : private internal::with_optional_allocator<Allocator> {
        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using allocator_type = Allocator;
            using value_type = CharType;

        private:
            CharType *data = nullptr;
            count_t _length = 0;
            allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS;

        public:
            dynamic_nul_terminated_string() : alloc{nullptr} {}

            template<
                typed_allocator<CharType> Allocator2
            > dynamic_nul_terminated_string(
                const basic_string<CharType, CharTraits, Allocator2> &str,
                allocator_type * const allocator,
                const allocation_flags alloc_flags
            ) :
                alloc{allocator},
                data{allocator->template allocate<value_type>(size_of<value_type>(str.length()), alloc_flags)},
                _length{str.length()},
                alloc_flags{alloc_flags}
            {
                str.to_nul_terminated(data);
            }

            dynamic_nul_terminated_string(
                const basic_string<CharType, CharTraits, Allocator> &str
            ) : dynamic_nul_terminated_string{str, str.get_allocator(), str.get_allocation_flags()}
            {}

            ~dynamic_nul_terminated_string() {
                destroy();
            }

            constexpr count_t length() const noexcept { return _length; }
            constexpr const CharType *value() const noexcept { return data; }

            constexpr void destroy() {
                if(data) {
                    alloc::get_allocator()->deallocate(data);
                    data = nullptr;
                    _length = 0;
                }
            }
    };

    template<count_t MaxLength = CCL_DEFAULT_NUL_TERMINATED_STRING_MAX_LENGTH>
    using ansi_nul_terminated_string = nul_terminated_string<char, MaxLength>;

    template<
        typed_allocator<char> Allocator = allocator
    > using ansi_dynamic_nul_terminated_string = dynamic_nul_terminated_string<
        typename ansi_string<Allocator>::value_type,
        typename ansi_string<Allocator>::char_Traits,
        Allocator
    >;
}

#endif // CCL_STRING_NUL_TERMINATED_HPP
