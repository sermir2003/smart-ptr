#pragma once

#include <type_traits>
#include <utility>

template <typename F, typename S>
constexpr bool kFirstCanBeCompressed =
    std::is_empty_v<F> && !std::is_final_v<F> && !std::is_base_of_v<F, S>;

template <typename F, typename S>
constexpr bool kSecondCanBeCompressed = std::is_empty_v<S> && !std::is_final_v<S>;

template <typename F, typename S, bool cmp_first = kFirstCanBeCompressed<F, S>,
          bool cmp_second = kSecondCanBeCompressed<F, S>>
class CompressedPair;

template <typename F, typename S>
class CompressedPair<F, S, true, true> : F, S {
public:
    CompressedPair() = default;
    template <typename U1, typename U2>
    CompressedPair(U1&& first, U2&& second)
        : F(std::forward<U1>(first)), S(std::forward<U2>(second)) {
    }
    F& First() {
        return *dynamic_cast<F*>(this);
    }
    const F& First() const {
        return *dynamic_cast<F*>(this);
    };
    S& Second() {
        return *dynamic_cast<S*>(this);
    }
    const S& Second() const {
        return *dynamic_cast<S*>(this);
    };
};

template <typename F, typename S>
class CompressedPair<F, S, true, false> : F {
public:
    CompressedPair() : second_() {
    }
    template <typename U1, typename U2>
    CompressedPair(U1&& first, U2&& second)
        : F(std::forward<U1>(first)), second_(std::forward<U2>(second)) {
    }
    F& First() {
        return *dynamic_cast<F*>(this);
    }
    const F& First() const {
        return *dynamic_cast<F*>(this);
    };
    S& Second() {
        return second_;
    }
    const S& Second() const {
        return second_;
    };

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, true> : S {
public:
    CompressedPair() : first_() {
    }
    template <typename U1, typename U2>
    CompressedPair(U1&& first, U2&& second)
        : first_(std::forward<U1>(first)), S(std::forward<U2>(second)) {
    }
    F& First() {
        return first_;
    }
    const F& First() const {
        return first_;
    };
    S& Second() {
        return *this;
    }
    const S& Second() const {
        return *this;
    };

private:
    F first_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, false> {
public:
    CompressedPair() : first_(), second_() {
    }
    template <typename U1, typename U2>
    CompressedPair(U1&& first, U2&& second)
        : first_(std::forward<U1>(first)), second_(std::forward<U2>(second)) {
    }
    F& First() {
        return first_;
    }
    const F& First() const {
        return first_;
    };
    S& Second() {
        return second_;
    }
    const S& Second() const {
        return second_;
    };

private:
    F first_;
    S second_;
};
