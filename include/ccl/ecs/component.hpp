/**
 * @file
 *
 * Type-erased ECS component definition.
 */
#ifndef CCL_ECS_COMPONENT_HPP
#define CCL_ECS_COMPONENT_HPP

#include <utility>
#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/paged-vector.hpp>

namespace ccl::ecs {
    template<typename T, typed_allocator<T> Allocator>
    class component;

    struct component_i {
        virtual ~component_i() = default;

        /**
         * @return The number of values in this component collection.
         */
        virtual std::size_t size() const = 0;

        /**
         * Set an existing component's value.
         *
         * @param index The index of the value to assign.
         * @param value The value to assign.
         */
        virtual void set(const std::size_t index, const void * const value) = 0;

        /**
         * Push a new value at the end of the collection.
         *
         * @param value The value to push back.
         */
        virtual void push_back(const void * const value) = 0;

        /**
         * Push a new value from the given component to the end of this collection.
         *
         * @param index The index of the value in `other` to push back.
         * @param other The other component collection to get the value from.
         */
        virtual void push_back_from(const std::size_t index, component_i& other) = 0;

        /**
         * Emplace an empty value at the end of this collection.
         */
        virtual void emplace_empty() = 0;

        /**
         * Erase a value from this collection.
         *
         * @param index The index of the value to remove.
         */
        virtual void erase(const std::size_t index) = 0;

        /**
         * Move an item from another collection.
         *
         * @param other The collection to move the item from.
         * @param index_from The index in `other` of the item to move.
         * @param index_to The index to the existing item in this collection to move the value to.
         */
        virtual void move_from(
            const component_i& other,
            const std::size_t index_from,
            const std::size_t index_to
        ) = 0;

        /**
         * Move an item within this collection.
         *
         * @param index_from The index of the item to move.
         * @param index_to The index of the destination item.
         */
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
    class component final : public component_i {
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

            virtual void move_from(
                const component_i& other,
                const std::size_t index_from,
                const std::size_t index_to
            ) override {
                const component& typed_other = other.template cast<T, Allocator>();

                items[index_to] = std::move(typed_other.items[index_from]);
            }

            virtual void move(const std::size_t index_from, const std::size_t index_to) override {
                items[index_to] = std::move(items[index_from]);
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
