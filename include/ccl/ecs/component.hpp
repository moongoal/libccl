/**
 * @file
 *
 * ECS component definition.
 */
#ifndef CCL_ECS_COMPONENT_HPP
#define CCL_ECS_COMPONENT_HPP

#include <memory>
#include <limits>
#include <ccl/api.hpp>
#include <ccl/type-traits.hpp>
#include <ccl/debug.hpp>
#include <ccl/paged-vector.hpp>
#include <ccl/memory/allocator.hpp>

namespace ccl::ecs {
    template<basic_allocator Allocator>
    class component {
        public:
            using allocator_type = Allocator;

            template<typename T>
            using item_collection = paged_vector<T, T*, allocator_type>;

        private:
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
                * Resize the component collection.
                *
                * @param new_size The new size of the collection.
                */
                virtual void resize(const std::size_t new_size) = 0;

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
                virtual iface* clone_empty(allocator_type * const allocator) const = 0;
            };

            template<typename T>
            struct typed_component final : public iface {
                using item_collection = component::item_collection<T>;

                item_collection items;

                constexpr typed_component(allocator_type * const allocator = nullptr)
                    : items{allocator}
                {}

                constexpr typed_component(const typed_component& other)
                    : items{other.items}
                {}

                constexpr typed_component(typed_component&& other)
                    : items{std::move(other.items)}
                {}

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
                    items.emplace_back();
                }

                virtual void resize(const std::size_t new_size) override {
                    items.resize(new_size);
                }

                virtual void erase(const std::size_t index) override {
                    items.erase(items.begin() + index);
                }

                virtual void move(const std::size_t index_from, const std::size_t index_to) override {
                    items[index_to] = items[index_from];
                }

                virtual iface* clone_empty(allocator_type * const allocator) const override {
                    return new typed_component{allocator};
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

        public:
            using id_type = std::size_t;

            static constexpr id_type invalid_id = std::numeric_limits<id_type>::max();

        private:
            id_type _id;
            std::unique_ptr<iface> ptr;

            template<typename T>
            auto& typed_ref() {
                return typed_component<std::remove_cvref_t<T>>::typed_ref(*ptr.get());
            }

            template<typename T>
            const auto& typed_ref() const {
                return typed_component<std::remove_cvref_t<T>>::typed_ref(*ptr.get());
            }

            constexpr component() : _id{invalid_id} {}
            constexpr component(const id_type id, iface* const ptr) : _id{id}, ptr{ptr} {}

            template<typename T>
            constexpr component(typed_component<T>* const ptr) : _id{make_id<T>()}, ptr{ptr} {}

        public:
            constexpr component(const component&) = delete;
            constexpr component(component&& other) : _id{other._id}, ptr{std::move(other.ptr)} {}

            template<typename T>
            static component make(allocator_type * const allocator = nullptr) {
                return component {
                    make_id<T>(),
                    new typed_component<std::remove_cvref_t<T>>{allocator}
                };
            }

            std::size_t size() const {
                return ptr.get()->size();
            }

            template<typename T>
            void set(const std::size_t index, const T& value) {
                if constexpr(std::is_pointer_v<T> || std::is_null_pointer_v<T>) {
                    return set(index, static_cast<const void*>(value));
                } else {
                    typed_ref<T>().set(index, value);
                }
            }

            void set(const std::size_t index, const void * const value) {
                CCL_THROW_IF(!value, std::invalid_argument{"Value cannot be null."});

                ptr.get()->set(index, value);
            }

            template<typename T>
            void push_back(const T& value) {
                if constexpr(std::is_pointer_v<T> || std::is_null_pointer_v<T>) {
                    return push_back(static_cast<const void*>(value));
                } else {
                    typed_ref<T>().push_back(value);
                }
            }

            void push_back(const void * const value) {
                CCL_THROW_IF(!value, std::invalid_argument{"Value cannot be null."});

                ptr.get()->push_back(value);
            }

            void push_back_from(const std::size_t index, component& other) {
                ptr.get()->push_back_from(index, *other.ptr.get());
            }

            template<typename T>
            void emplace_empty() {
                typed_ref<T>().emplace_empty();
            }

            void emplace_empty() {
                ptr.get()->emplace_empty();
            }

            template<typename T>
            void resize(const std::size_t new_size) {
                typed_ref<T>().resize(new_size);
            }

            void resize(const std::size_t new_size) {
                ptr.get()->resize(new_size);
            }

            void erase(const std::size_t index) {
                ptr.get()->erase(index);
            }

            void move(const std::size_t index_from, const std::size_t index_to) {
                ptr.get()->move(index_from, index_to);
            }

            component clone_empty(allocator_type * const allocator = nullptr) const {
                return component{_id, ptr.get()->clone_empty(allocator)};
            }

            void move_from(
                const component& other,
                const std::size_t index_from,
                const std::size_t index_to
            ) {
                ptr.get()->move_from(*other.ptr.get(), index_from, index_to);
            }

            /**
             * Get a value from the component.
             *
             * @tparam T The component value type.
             *
             * @param index The index of the value to retrieve.
             *
             * @return The component value at the given index.
             */
            template<typename T>
            constexpr T& get(const std::size_t index) {
                return typed_ref<T>().items[index];
            }

            /**
             * Get a value from the component.
             *
             * @tparam T The component value type.
             *
             * @param index The index of the value to retrieve.
             *
             * @return The component value at the given index.
             */
            template<typename T>
            constexpr const T& get(const std::size_t index) const {
                return typed_ref<T>().items[index];
            }

            /**
             * Get the component vector.
             *
             * @tparam T The component value type.
             *
             * @return The a reference to the component vector.
             */
            template<typename T>
            constexpr const item_collection<T>& get() const {
                return typed_ref<T>().items;
            }

            /**
             * Get the ID of a typed component.
             *
             * @tparam T The component data type.
             *
             * @return The ID of the given component type.
             */
            template<typename T>
            static constexpr id_type make_id() {
                using base_t = std::remove_cvref_t<T>;

                return typeid(typed_component<base_t>).hash_code();
            }

            /**
             * Get the component ID. All components of the same underlying type
             * will have the same ID.
             *
             * @return The unique, type-based component ID.
             */
            constexpr id_type id() const {
                return _id;
            }
    };
}

#endif // CCL_ECS_COMPONENT_HPP
