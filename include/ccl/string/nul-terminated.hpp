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
    template<
        typename CharType,
        char_traits_impl<CharType> CharTraits = char_traits<CharType>,
        typed_allocator<CharType> Allocator = allocator
    > class nul_terminated_string : private internal::with_optional_allocator<Allocator> {
        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using allocator_type = Allocator;
            using value_type = CharType;
            using char_traits = CharTraits;

            static constexpr count_t max_local_storage_length = 16;

        private:
            CharType *data = nullptr;
            count_t _length = 0;
            allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS;
            CharType local_storage[max_local_storage_length];

        public:
            nul_terminated_string() : alloc{nullptr}, data{local_storage} { char_traits::assign(local_storage[0], char_traits::nul()); }

            template<
                typed_allocator<CharType> Allocator2
            > nul_terminated_string(
                const basic_string<CharType, CharTraits, Allocator2> &str,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) :
                alloc{allocator},
                data{
                    str.length() < max_local_storage_length
                    ? data = local_storage
                    : alloc::get_allocator()->template allocate<value_type>(size_of<value_type>(str.length() + 1), alloc_flags)
                },
                _length{str.length()},
                alloc_flags{alloc_flags}
            {
                str.to_nul_terminated(std::span{data, _length + 1});
            }

            nul_terminated_string(
                const basic_string<CharType, CharTraits, Allocator> &str
            ) : nul_terminated_string{str, str.get_allocator(), str.get_allocation_flags()}
            {}

            ~nul_terminated_string() {
                destroy();
            }

            constexpr count_t length() const noexcept { return _length; }

            constexpr const CharType *value() const noexcept {
                return data;
            }

            constexpr void destroy() {
                if(and_(data != nullptr, data != local_storage)) {
                    alloc::get_allocator()->deallocate(data);
                    data = local_storage;
                    _length = 0;
                    char_traits::assign(local_storage[0], char_traits::nul());
                }
            }

            constexpr nul_terminated_string& operator =(const nul_terminated_string &rhs) {
                destroy();

                alloc::operator=(rhs.get_allocator());
                alloc_flags = rhs.get_allocation_flags();

                if(rhs.data) {
                    data = (
                        rhs._length < max_local_storage_length
                        ? data = local_storage
                        : alloc::get_allocator()->template allocate<value_type>(
                            size_of<value_type>(rhs.length()),
                            alloc_flags
                        )
                    );

                    _length = rhs._length;

                    char_traits::copy(data, rhs.data, _length);
                }

                return *this;
            }

            constexpr bool is_empty() const noexcept { return _length == 0; }
            constexpr allocator_type* get_allocator() const noexcept { return alloc::get_allocator(); }
            constexpr allocation_flags get_allocation_flags() const noexcept { return alloc_flags; }
    };

    template<
        typed_allocator<char> Allocator = allocator
    > using ansi_nul_terminated_string = nul_terminated_string<
        typename ansi_string<Allocator>::value_type,
        typename ansi_string<Allocator>::char_traits,
        Allocator
    >;
}

#endif // CCL_STRING_NUL_TERMINATED_HPP
