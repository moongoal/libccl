/**
 * @file
 *
 * An iterator for contiguous, linear memory.
 */
#ifndef CCL_CONTIGUOUS_ITERATOR_HPP
#define CCL_CONTIGUOUS_ITERATOR_HPP

#include <iterator>
#include <ccl/api.hpp>

namespace ccl {
    template<typename T>
    struct contiguous_iterator {
        using iterator_category = std::contiguous_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;

        constexpr contiguous_iterator(const pointer ptr = nullptr) noexcept : ptr{ptr} {}

        template<typename U>
        constexpr contiguous_iterator(const contiguous_iterator<U> &other) noexcept : ptr{other.ptr} {}

        constexpr contiguous_iterator(const contiguous_iterator &other) noexcept : ptr{other.ptr} {}

        constexpr contiguous_iterator& operator =(const contiguous_iterator &other) noexcept {
            ptr = other.ptr;

            return *this;
        }

        constexpr reference operator*() const noexcept { return *ptr; }
        constexpr pointer operator->() const noexcept { return ptr; }

        constexpr contiguous_iterator& operator +=(const difference_type n) noexcept {
            ptr += n;
            return *this;
        }

        constexpr contiguous_iterator& operator -=(const difference_type n) noexcept {
            ptr -= n;
            return *this;
        }

        constexpr contiguous_iterator operator +(const difference_type n) const noexcept {
            return ptr + n;
        }

        constexpr contiguous_iterator operator -(const difference_type n) const noexcept {
            return ptr - n;
        }

        constexpr difference_type operator -(const contiguous_iterator other) const noexcept {
            return ptr - other.ptr;
        }

        constexpr reference operator[](const difference_type i) const noexcept {
            return ptr[i];
        }

        constexpr contiguous_iterator& operator --() noexcept {
            --ptr;
            return *this;
        }

        constexpr contiguous_iterator operator --(int) noexcept {
            return ptr--;
        }

        constexpr contiguous_iterator& operator ++() noexcept {
            ++ptr;
            return *this;
        }

        constexpr contiguous_iterator operator ++(int) noexcept {
            return ptr++;
        }

        pointer ptr;
    };

    template<typename T>
    constexpr bool operator ==(const contiguous_iterator<T> &a, const contiguous_iterator<T> &b) noexcept {
        return a.ptr == b.ptr;
    }

    template<typename T>
    constexpr bool operator !=(const contiguous_iterator<T> &a, const contiguous_iterator<T> &b) noexcept {
        return a.ptr != b.ptr;
    }

    template<typename T>
    constexpr bool operator >(const contiguous_iterator<T> &a, const contiguous_iterator<T> &b) noexcept {
        return a.ptr > b.ptr;
    }

    template<typename T>
    constexpr bool operator <(const contiguous_iterator<T> &a, const contiguous_iterator<T> &b) noexcept {
        return a.ptr < b.ptr;
    }

    template<typename T>
    constexpr bool operator >=(const contiguous_iterator<T> &a, const contiguous_iterator<T> &b) noexcept {
        return a.ptr >= b.ptr;
    }

    template<typename T>
    constexpr bool operator <=(const contiguous_iterator<T> &a, const contiguous_iterator<T> &b) noexcept {
        return a.ptr <= b.ptr;
    }

    template<typename T>
    static constexpr contiguous_iterator<T> operator +(
        const typename contiguous_iterator<T>::difference_type n,
        const contiguous_iterator<T> it
    ) noexcept {
        return contiguous_iterator<T>{it.data() + n};
    }
}

#endif // CCL_CONTIGUOUS_ITERATOR_HPP
