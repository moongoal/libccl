/**
 * @file
 *
 * ECS component definition.
 */
#ifndef CCL_ECS_COMPONENT2_HPP
#define CCL_ECS_COMPONENT2_HPP

#include <memory>
#include <ccl/api.hpp>
#include <ccl/type-traits.hpp>
#include <ccl/paged-vector.hpp>
#include <ccl/memory/allocator.hpp>

namespace ccl::ecs {
    template<basic_allocator Allocator>
    class component2 {
        struct iface {
            virtual ~iface() = default;

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
            virtual void push_back_from(const std::size_t index, iface& other) = 0;

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
                const iface& other,
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

            /**
             * Create an empty clone of this component. The new component
             * is heap-allocated via `new`.
             *
             * @return The pointer to the newly created clone.
             */
            virtual iface* clone_empty() = 0;
        };

        template<typename T>
        struct typed_component final : public iface {
            using item_collection = paged_vector<T, T*, Allocator>;

            item_collection items;

            virtual ~typed_component() = default;

            virtual std::size_t size() const override {
                return items.size();
            }

            void set(const std::size_t index, const T& value) {
                items[index] = value;
            }

            virtual void set(const std::size_t index, const void * const value) override {
                items[index] = *reinterpret_cast<const T*>(value);
            }

            void push_back(const T& value) {
                items.push_back(value);
            }

            virtual void push_back(const void * const value) override {
                items.push_back(*reinterpret_cast<const T*>(value));
            }

            virtual void push_back_from(const std::size_t index, iface& other) override {
                items.push_back(
                    typed_component::typed_ref(other).items[index]
                );
            }

            virtual void emplace_empty() override {
                items.emplace();
            }

            virtual void erase(const std::size_t index) override {
                items.erase(items.begin() + index);
            }

            virtual void move(const std::size_t index_from, const std::size_t index_to) override {
                items[index_to] = items[index_from];
            }

            virtual iface* clone_empty() override {
                return new typed_component;
            }

            virtual void move_from(
                const iface& other,
                const std::size_t index_from,
                const std::size_t index_to
            ) override {
                items[index_to] = typed_ref(other).items[index_from];
            }

            static constexpr auto& typed_ref(iface& other) {
                return

                    #ifdef CCL_FEATURE_TYPECHECK_CASTS
                        dynamic_cast
                    #else // CCL_FEATURE_TYPECHECK_CASTS
                        reinterpret_cast
                    #endif // CCL_FEATURE_TYPECHECK_CASTS

                    <typed_component<std::remove_cvref_t<T>>&>(other);
            }

            static constexpr const auto& typed_ref(const iface& other) {
                return

                    #ifdef CCL_FEATURE_TYPECHECK_CASTS
                        dynamic_cast
                    #else // CCL_FEATURE_TYPECHECK_CASTS
                        reinterpret_cast
                    #endif // CCL_FEATURE_TYPECHECK_CASTS

                    <const typed_component<std::remove_cvref_t<T>>&>(other);
            }
        };

        std::unique_ptr<iface> ptr;

        template<typename T>
        auto& typed_ref() {
            return typed_component<std::remove_cvref_t<T>>::typed_ref(*ptr.get());
        }

        constexpr component2() = default;
        constexpr component2(iface* const ptr) : ptr{ptr} {}

        template<typename T>
        constexpr component2(typed_component<T>* const ptr) : ptr{ptr} {}

        public:
            constexpr component2(const component2&) = delete;
            constexpr component2(component2&& other) : ptr{std::move(other.ptr)} {}

            template<typename T>
            static component2 make() {
                return component2 {
                    new typed_component<std::remove_cvref_t<T>>
                };
            }

            std::size_t size() const {
                return ptr.get()->size();
            }

            template<typename T>
            void set(const std::size_t index, const T& value) {
                if constexpr(std::is_pointer_v<T>) {
                    return set(index, reinterpret_cast<const void*>(value));
                } else {
                    typed_ref<T>().set(index, value);
                }
            }

            void set(const std::size_t index, const void * const value) {
                ptr.get()->set(index, value);
            }

            template<typename T>
            void push_back(const T& value) {
                if constexpr(std::is_pointer_v<T>) {
                    return push_back(reinterpret_cast<const void*>(value));
                } else {
                    typed_ref<T>().push_back(value);
                }
            }

            void push_back(const void * const value) {
                ptr.get()->push_back(value);
            }

            void push_back_from(const std::size_t index, component2& other) {
                ptr.get()->push_back_from(index, *other.ptr.get());
            }

            template<typename T>
            void emplace_empty() {
                typed_ref<T>().emplace_empty();
            }

            void emplace_empty() {
                ptr.get()->emplace_empty();
            }

            void erase(const std::size_t index) {
                ptr.get()->erase(index);
            }

            void move(const std::size_t index_from, const std::size_t index_to) {
                ptr.get()->move(index_from, index_to);
            }

            component2 clone_empty() {
                return component2{ptr.get()->clone_empty()};
            }

            void move_from(
                const component2& other,
                const std::size_t index_from,
                const std::size_t index_to
            ) {
                ptr.get()->move_from(*other.ptr.get(), index_from, index_to);
            }

            template<typename T>
            constexpr T& get(const std::size_t index) {
                return typed_ref<T>().items[index];
            }
    };
}

#endif // CCL_ECS_COMPONENT2_HPP
