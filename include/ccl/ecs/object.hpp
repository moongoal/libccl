/**
 * @file
 *
 * ECS object definition.
 */
#ifndef CCL_ECS_OBJECT_HPP
#define CCL_ECS_OBJECT_HPP

#include <utility>
#include <ccl/api.hpp>

namespace ccl::ecs {
    template<typename T>
    class object;

    struct generic_object {
        virtual ~generic_object() = default;

        virtual generic_object& operator =(const generic_object &other) = 0;
        virtual generic_object& operator =(generic_object &&other) = 0;

        template<typename T>
        constexpr object<T>& cast() {
            return *

            #ifdef CCL_FEATURE_TYPECHECK_CASTS
                dynamic_cast
            #else // CCL_FEATURE_TYPECHECK_CASTS
                reinterpret_cast
            #endif // CCL_FEATURE_TYPECHECK_CASTS

            <object<T>*>(this);
        }

        template<typename T>
        constexpr const object<T>& cast() const {
            return *

            #ifdef CCL_FEATURE_TYPECHECK_CASTS
                dynamic_cast
            #else // CCL_FEATURE_TYPECHECK_CASTS
                reinterpret_cast
            #endif // CCL_FEATURE_TYPECHECK_CASTS

            <const object<T>*>(this);
        }
    };

    template<typename T>
    class object : public generic_object {
        T value;

        public:
            constexpr object() = default;
            constexpr object(const T& value) : value{value} {}
            constexpr object(const T&& value) : value{std::move(value)} {}
            virtual ~object() = default;

            virtual generic_object& operator =(const generic_object &other) override {
                const object<T>& obj = other.cast<T>();

                value = obj.value;

                return *this;
            }

            virtual generic_object& operator =(generic_object &&other) override {
                decltype(auto) obj = reinterpret_cast<object<T>&&>(other);

                value = std::move(obj.value);

                return *this;
            }

            constexpr T& get() { return value; }
            constexpr const T& get() const { return value; }
    };
}

#endif // CCL_ECS_OBJECT_HPP
