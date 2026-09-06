//#pragma once
//
//#include <array>
//#include <span>
//#include <algorithm>
//#include <numeric>
//
///**
// * @brief Allows to store any length string in a stack allocated char array.
// * However, there is no indexing, and stored strings cannot be modified.
// * 
// * @tparam CharCount Amount of maximum characters that can be stored.
// * @tparam MinSize Minimal size of stored strings.
// */
//template <typename Ty, size_t Elements, size_t MinSize = 0>
//struct sdarr {
//    static_assert(MinSize < Elements, "Cannot store any array, reduce minimal size.");
//
//public:
//    constexpr std::span<Ty> operator[](size_t index) const noexcept {
//        const auto first = std::next(elements.begin(), begin_of(index));
//        return { first, lengths[index] };
//    }
//
//    /**
//     * @brief Extend current (last) string by other string.
//     * 
//     * @returns Whether extending was possible.
//     */
//    constexpr bool extend(std::span<const Ty> content) noexcept {
//        const size_t offset = begin_of(current) + lengths[current];
//        const auto first = std::next(elements.begin(), offset);
//        if (offset + content.size() >= CharCount) {
//            return false;
//        }
//
//        lengths[current] += content.size();
//        std::copy_n(content.begin(), content.size(), first);
//        return true;
//    }
//
//    constexpr size_t size() const noexcept {
//        return current + 1;
//    }
//
//    constexpr bool next() noexcept {
//        lengths[current] = std::max(lengths[current], MinSize);
//        return (++current < lengths.size());
//    }
//
//private:
//    constexpr size_t begin_of(size_t index) const noexcept {
//        const auto begin = lengths.begin();
//        return std::accumulate(begin, std::next(begin, index), size_t{});
//    }
//
//public:
//    std::array<Ty, CharCount> elements;
//    std::array<size_t, CharCount / std::max(MinSize, size_t{1})> lengths{};
//    size_t current{};
//};