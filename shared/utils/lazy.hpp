#include <atomic>
#include <cstdint>
#include <functional>
#include <optional>

/// @brief Thread-safe container which ensures its value is initialized only once.
/// @tparam T Type of the contained value.
template <typename T>
class Once {
    std::optional<T> value;
    std::atomic_uintptr_t state_and_queue;

    void initialize(std::function<T()> const& initializer);

  public:
    /// @brief Constructs an uninitialized `Once<T>`.
    constexpr Once() : value(), state_and_queue(0) {}

    Once(Once<T> const&) = delete;

    /// @brief Gets the current value, if it is initialized.
    std::optional<T const&> get() const {
        return this->value;
    }
    //// @brief Gets the current value, if it is initialized.
    std::optional<T&> get() {
        return this->value;
    }

    /// @brief Gets the current value, or initializes it.
    /// @param initializer Function to use for initialization if required.
    T& get_or_initialize(std::function<T()> const& initializer) {
        this->initialize(initializer);
        return *this->value;
    }
};

/// @brief Thread-safe, lazy-initialized value.
/// @tparam T Type of the value.
/// @tparam F Type of the initializer.
template <typename T, typename F = std::function<T()>>
class Lazy {
    mutable Once<T> inner;
    F initializer;

  public:
    /// @brief Construces an uninitialized `Lazy<T>`.
    /// @param initializer_ Function to use for initialization on the first access.
    Lazy(F const& initializer_) : inner(), initializer(initializer_) {}

    T const* operator->() const {
        return this->inner.get_or_initialize(initializer);
    }
    T* operator->() {
        return this->inner.get_or_initialize(initializer);
    }

    T const& operator*() const {
        return this->inner.get_or_initialize(initializer);
    }
    T& operator*() {
        return this->inner.get_or_initialize(initializer);
    }
};
