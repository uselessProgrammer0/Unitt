#pragma once

#include <array>

/**
 * @brief Simplified pseudo dynamic array.
 * This means that it behaves like pseudo dynamic array (pdarray), but it doesn't really bother all the initialization semantics.
 * Elements are default initialized when this array is default initialized.
 */
template <typename Ty, size_t Size>
class spdarr {
public:
    [[nodiscard]] constexpr Ty& operator[](size_t index) noexcept {
        return elems[index];
    }

    [[nodiscard]] constexpr const Ty& operator[](size_t index) const noexcept {
        return elems[index];
    }

    [[nodiscard]] constexpr Ty& front() noexcept {
        // TODO: Error handling.
        return elems[0];
    }

    [[nodiscard]] constexpr const Ty& front() const noexcept {
        // TODO: Error handling.
        return elems[0];
    }

    constexpr void push_back(const Ty& value) noexcept {
        // TODO: Error handling.
        elems[csize++] = value;
    }

    constexpr void push_back(Ty&& value) noexcept {
        // TODO: Error handling.
        elems[csize++] = std::move(value);
    }

    [[nodiscard]] constexpr Ty& back() noexcept {
        // TODO: Error handling.
        return elems[csize - 1];
    }

    [[nodiscard]] constexpr const Ty& back() const noexcept {
        // TODO: Error handling.
        return elems[csize - 1];
    }

    [[nodiscard]] constexpr size_t size() const noexcept {
        return csize;
    }

public:
    std::array<Ty, Size> elems{};
    size_t csize{};
};