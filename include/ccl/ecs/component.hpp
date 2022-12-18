/**
 * @file
 *
 * Type-erased ECS component definition.
 */
#ifndef CCL_ECS_COMPONENT_HPP
#define CCL_ECS_COMPONENT_HPP

#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/paged-vector.hpp>

namespace ccl::ecs {
    template<typename T, typed_allocator<T> Allocator>
    class component;

    struct component_i {
        virtual ~component_i() = default;

        virtual std::size_t size() const = 0;
        virtual void set(const std::size_t index, const void * const value) = 0;
        virtual void push_back(const void * const value) = 0;
        virtual void push_back_from(const std::size_t index, component_i& other) = 0;
        virtual void emplace_empty() = 0;
        virtual void erase(const std::size_t index) = 0;

        virtual void copy_from(
            const component_i& other,
            const std::size_t index_from,
            const std::size_t index_to
        ) = 0;

        virtual void move(const std::size_t index_from, const std::size_t index_to) = 0;

        template<typename T, typed_allocator<T> Allocator>
        constexpr component<T, Allocator>& cast() {
            return *

            #ifdef CCL_FEATURE_TYPECHECK_CASTS
                dynamic_cast
            #else // CCL_FEATURE_TYPECHECK_CASTS
                reinterpret_cast
            #endif // CCL_FEATURE_TYPECHECK_CASTS

            <component<T, Allocator>*>(this);
        }

        template<typename T, typed_allocator<T> Allocator>
        constexpr const component<T, Allocator>& cast() const {
            return *

            #ifdef CCL_FEATURE_TYPECHECK_CASTS
                dynamic_cast
            #else // CCL_FEATURE_TYPECHECK_CASTS
                reinterpret_cast
            #endif // CCL_FEATURE_TYPECHECK_CASTS

            <const component<T, Allocator>*>(this);
        }
    };

    template<typename T, typed_allocator<T> Allocator>
    class component : public component_i {
        public:
            using value_type = T;
            using allocator_type = Allocator;
            using item_collection = paged_vector<T, T*, Allocator>;

        private:
            item_collection items;

            virtual void set(const std::size_t index, const void * const value) override {
                CCL_THROW_IF(!value, std::invalid_argument{"Value cannot be null."});

                const T& typed_value = *reinterpret_cast<const T*>(value);

                items[index] = typed_value;
            }

            virtual void push_back(const void * const value) override {
                CCL_THROW_IF(!value, std::invalid_argument{"Value cannot be null."});

                const T& typed_value = *reinterpret_cast<const T*>(value);

                items.push_back(typed_value);
            }

        public:
            constexpr auto& get() { return items; }
            constexpr const auto& get() const { return items; }

            virtual std::size_t size() const override {
                return items.size();
            }

            void set(const std::size_t index, const T& value) {
                set(index, &value);
            }

            void push_back(const T& value) {
                push_back(&value);
            }

            virtual void push_back_from(const std::size_t index, component_i& other) override {
                component& typed_other = other.template cast<T, Allocator>();

                items.push_back(typed_other.items[index]);
            }

            virtual void emplace_empty() override {
                items.emplace();
            }

            virtual void copy_from(
                const component_i& other,
                const std::size_t index_from,
                const std::size_t index_to
            ) override {
                const component& typed_other = other.template cast<T, Allocator>();

                items[index_to] = typed_other.items[index_from];
            }

            virtual void move(const std::size_t index_from, const std::size_t index_to) override {
                items[index_to] = items[index_from];
            }

            virtual void erase(const std::size_t index) override {
                items.erase(items.begin() + index);
            }

            constexpr std::size_t id() const {
                return typeid(decltype(*this)).hash_code();
            }
    };
}

#endif // CCL_ECS_COMPONENT_HPP
