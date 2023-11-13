#ifndef FREESTANDING_MERSENNE_TWISTER_H
#define FREESTANDING_MERSENNE_TWISTER_H 1'01

namespace freestanding_mersenne_twister {

using size_t = decltype(sizeof(char));

namespace detail {

#if __cpp_constexpr >= 201304L
    template<class UInt>
    constexpr size_t bits_in_type() noexcept {
        UInt x = ~UInt{0};
        size_t bits = 0;
        while (x) {
            ++bits;
            x >>= 1;
        }
        return bits;
    }
#else
    template<class UInt>
    constexpr size_t bits_in_type(UInt x = ~UInt{0}, size_t bits = 0) noexcept {
        return x ? bits_in_type(x >>= 1, ++bits) : bits;
    }
#endif

#ifndef FREESTANDING_MERSENNE_TWISTER_NO_SEED_SEQUENCE
    template<bool B> struct bool_constant { static constexpr bool value = B; };

    template<class T>
    constexpr bool is_unsigned_integer_of_at_least_32_bits() noexcept {
        return (T{0} == T{}) && (T(0) == T{}) && (T{0u} == T()) &&
            (static_cast<T>(0) == T()) && (static_cast<T>(0) == 0u) &&
            (static_cast<T>(-1) > T(0)) && (static_cast<unsigned long>(static_cast<T>(0xffff'ffffu)) == 0xffff'ffffu);
    }

    template<class T>
    constexpr bool is_unsigned_integer_of_at_least_64_bits() noexcept {
        return is_unsigned_integer_of_at_least_32_bits<T>() && (static_cast<unsigned long long>(static_cast<T>(0xffff'ffff'ffff'ffffu)) == 0xffff'ffff'ffff'ffffu);
    }

    template<bool B, class IfTrue, class IfFalse> struct conditional { using type = IfFalse; };
    template<class IfTrue, class IfFalse> struct conditional<true, IfTrue, IfFalse> { using type = IfTrue; };

    template<class... Candidates> struct smallest_32_bit_type { using type = unsigned long; };
    template<class Candidate, class... Candidates>
    struct smallest_32_bit_type<Candidate, Candidates...> : conditional<is_unsigned_integer_of_at_least_32_bits<Candidate>(), Candidate, typename smallest_32_bit_type<Candidates...>::type> {};

    template<class... Candidates> struct smallest_64_bit_type { using type = unsigned long long; };
    template<class Candidate, class... Candidates>
    struct smallest_64_bit_type<Candidate, Candidates...> : conditional<is_unsigned_integer_of_at_least_64_bits<Candidate>(), Candidate, typename smallest_64_bit_type<Candidates...>::type> {};

    using uint_least32_t = typename smallest_32_bit_type<unsigned short, unsigned>::type;
    using uint_least64_t = typename smallest_64_bit_type<unsigned short, unsigned, unsigned long>::type;

    template<class T, class U> struct is_same : bool_constant<false> {};
    template<class T> struct is_same<T, T> : bool_constant<true> {};
#if __cpp_concepts >= 201907L
    template<class T, class U> concept same_as = is_same<T, U>::value;

    template<class S>
    concept seed_sequence =
        requires { typename S::result_type; } &&
        bool_constant<is_unsigned_integer_of_at_least_32_bits<typename S::result_type>()>::value &&
        requires (S q, const S r, const unsigned long* cit, unsigned long* mit) {
            { S() };
            { S(cit, cit) };
            { S(mit, mit) };
            // { S{ 1ul, 1ul, 1ul, 1ul, 1ul, 1ul, 1ul } };
            { q.generate(mit, mit) } -> same_as<void>;
            { r.size() } -> same_as<size_t>;
            { q.size() } -> same_as<size_t>;
            { q.param(mit) } -> same_as<void>;
            { r.param(mit) } -> same_as<void>;
        };

    template<class T, class Enabled>
    struct enable_if_seed_sequence_impl {};

    template<seed_sequence T, class Enabled>
    struct enable_if_seed_sequence_impl<T, Enabled> { using type = Enabled; };

    template<class T, class Enabled = void> using enable_if_seed_sequence = typename enable_if_seed_sequence_impl<T, Enabled>::type;
#else
    template<bool B, class Enabled = void> struct enable_if { };
    template<class Enabled> struct enable_if<true, Enabled> { using type = Enabled; };
    template<class...> using void_t = void;

    template<
        class S,
        class T = typename S::result_type,
        enable_if<is_unsigned_integer_of_at_least_32_bits<T>()>* = nullptr,
        S* qp = nullptr, const S* rp = nullptr, const unsigned long* cit = nullptr, unsigned long* mit = nullptr,
        void_t<
            decltype(S()),
            decltype(S(cit, cit)),
            decltype(S(mit, mit))
            // decltype(S{ 1ul, 1ul, 1ul, 1ul, 1ul, 1ul, 1ul })
        >* = nullptr,
        enable_if<
            is_same<decltype(qp->generate(mit, mit)), void>::value &&
            is_same<decltype(rp->size()), size_t>::value &&
            is_same<decltype(qp->size()), size_t>::value &&
            is_same<decltype(qp->param(mit)), void>::value &&
            is_same<decltype(rp->param(mit)), void>::value
        >* = nullptr
    > static bool_constant<true> test(int);

    template<class>
    static bool_constant<false> test(...);

    template<class T, class Enabled = void> using enable_if_seed_sequence = typename enable_if<decltype(test<T>(0))::value, Enabled>::type;
#endif
#endif

}

template<class UIntType, size_t n>
struct mersenne_twister_state {
    using result_type = UIntType;

    static constexpr size_t state_size = n;

    // Invariants: Each X[i] will have at most "word_size"/w bits (i.e., `(X[i] >> w) == 0u`)
    //             i < n
    result_type X[state_size];
    size_t i;

#if __cpp_impl_three_way_comparison >= 201907L
    friend constexpr bool operator==(const mersenne_twister_state&, const mersenne_twister_state&) noexcept = default;
#else
    friend
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    bool operator==(const mersenne_twister_state& x, const mersenne_twister_state& y) noexcept {
        for (size_t i = 0; i < state_size; ++i) {
            if (x.X[i] != y.X[i]) return false;
        }
        return x.i == y.i;
    }
    friend
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    bool operator!=(const mersenne_twister_state& x, const mersenne_twister_state& y) noexcept {
        return !operator==(x, y);
    }
#endif

    template<class OStream>
    friend OStream& operator<<(OStream& os, const mersenne_twister_state& x) {
        const auto flags(os.flags());
        auto fill(os.fill());
        const auto space(os.widen(' '));
        os.flags(OStream::dec | OStream::fixed | OStream::left);
        os.fill(space);

        for (const result_type& x_i : x.X) {
            os << x_i;
            os << space;
        }
        os << x.i;

        os.flags(flags);
        os.fill(static_cast<decltype(fill)&&>(fill));
        return os;
    }

    template<class IStream>
    friend IStream& operator>>(IStream& is, mersenne_twister_state& x) {
        const auto flags(is.flags());
        is.flags(IStream::dec | IStream::skipws);

        for (result_type& x_i : x.X) {
            is >> x_i;
        }
        is >> x.i;

        is.flags(flags);
        return is;
    }
};

template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
struct mersenne_twister_engine : public mersenne_twister_state<UIntType, n> {
    using result_type = UIntType;
    static_assert(static_cast<result_type>(-1) >= result_type{0}, "result_type must be unsigned");
    using state_type = mersenne_twister_state<UIntType, n>;
    using state_type::X;
    using state_type::i;

    static constexpr size_t word_size = w;
    using state_type::state_size;
    static constexpr size_t shift_size = m;
    static constexpr size_t mask_bits = r;
    static constexpr UIntType xor_mask = a;
    static constexpr size_t tempering_u = u;
    static constexpr UIntType tempering_d = d;
    static constexpr size_t tempering_s = s;
    static constexpr UIntType tempering_b = b;
    static constexpr size_t tempering_t = t;
    static constexpr UIntType tempering_c = c;
    static constexpr size_t tempering_l = l;
    static constexpr UIntType initialization_multiplier = f;
    static constexpr result_type min() noexcept { return result_type{0u}; }
    static constexpr result_type max() noexcept { return max_value; }

private:
    static constexpr size_t result_type_bits = detail::bits_in_type<result_type>();
    static_assert(word_size <= result_type_bits, "result_type is too small for word_size bits");
    static constexpr result_type max_value{ (~result_type{0}) >> (result_type_bits - w) };

public:
    static constexpr result_type default_seed{ static_cast<result_type>(5489u) & max() };

    static_assert(0 < shift_size, "shift_size cannot be 0");
    static_assert(shift_size <= state_size, "shift_size not in range (replace with shift_size%state_size)");
    static_assert(2u < word_size, "word_size is too small (cannot be 0, 1 or 2)");
    static_assert(mask_bits <= word_size, "mask_bits too large (more than word_size)");
    static_assert(tempering_u <= word_size, "tempering_u too large (shifts more than word_size)");
    static_assert(tempering_s <= word_size, "tempering_s too large (shifts more than word_size)");
    static_assert(tempering_t <= word_size, "tempering_t too large (shifts more than word_size)");
    static_assert(tempering_l <= word_size, "tempering_l too large (shifts more than word_size)");
    static_assert(xor_mask <= max_value, "xor_mask has too many bits set");
    static_assert(tempering_b <= max_value, "tempering_b has too many bits set");
    static_assert(tempering_c <= max_value, "tempering_c has too many bits set");
    static_assert(tempering_d <= max_value, "tempering_d has too many bits set");
    static_assert(initialization_multiplier <= max_value, "initialization_multiplier too large");

#if __cpp_constexpr >= 201304L
    constexpr
#endif
    mersenne_twister_engine() noexcept : mersenne_twister_engine(default_seed) {}
    constexpr explicit mersenne_twister_engine(const state_type& state) noexcept : state_type(state) {}
    // Disable implicit mersenne_twister_engine<UIntType, w, <different parameters>> -> state_type converting constructor
    template<class UIntType2, size_t w2, size_t n2, size_t m2, size_t r2, UIntType a2, size_t u2, UIntType d2, size_t s2, UIntType b2, size_t t2, UIntType c2, size_t l2, UIntType f2>
    mersenne_twister_engine(const mersenne_twister_engine<UIntType2, w2, n2, m2, r2, a2, u2, d2, s2, b2, t2, c2, l2, f2>&) = delete;
    // Except if only the UIntType is different (e.g., `mt19937<unsigned>` -> `mt19937<unsigned long>`
    template<class UIntType2>
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    explicit
    mersenne_twister_engine(const mersenne_twister_engine<UIntType2, w, n, m, r, a, u, d, s, b, t, c, l, f>& other) noexcept : state_type{} {
        *this = other;
    }

#if __cpp_constexpr >= 201304L
    constexpr
#endif
    mersenne_twister_engine& operator=(const state_type& other) noexcept {
        static_cast<state_type&>(*this) = other;
        return *this;
    }

    template<class UIntType2, size_t w2, size_t n2, size_t m2, size_t r2, UIntType a2, size_t u2, UIntType d2, size_t s2, UIntType b2, size_t t2, UIntType c2, size_t l2, UIntType f2>
    mersenne_twister_engine& operator=(const mersenne_twister_engine<UIntType2, w2, n2, m2, r2, a2, u2, d2, s2, b2, t2, c2, l2, f2>&) = delete;
    template<class UIntType2>
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    mersenne_twister_engine& operator=(const mersenne_twister_engine<UIntType2, w, n, m, r, a, u, d, s, b, t, c, l, f>& other) noexcept {
        for (size_t i = 0; i < state_size; ++i) {
            X[i] = static_cast<UIntType>(other.X[i]);
        }
        i = other.i;
        return *this;
    }

#if __cpp_constexpr >= 201304L
    constexpr
#endif
    explicit mersenne_twister_engine(result_type value) noexcept : state_type{} { seed(value); }
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    void seed(result_type value = default_seed) noexcept {
        X[0] = value & max();
        for (size_t j = 1; j < state_size; ++j) {
            X[j] = (initialization_multiplier * (X[j-1u] xor (X[j-1u] >> (word_size - 2u))) + j) & max();
        }

        transition_algorithm();
        i = 0;
    }

#if __cpp_impl_three_way_comparison >= 201907L
    friend constexpr bool operator==(const mersenne_twister_engine&, const mersenne_twister_engine&) noexcept = default;
#else
    friend
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    bool operator==(const mersenne_twister_engine& x, const mersenne_twister_engine& y) noexcept {
        return static_cast<const state_type&>(x) == static_cast<const state_type&>(y);
    }
    friend
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    bool operator!=(const mersenne_twister_engine& x, const mersenne_twister_engine& y) noexcept {
        return !operator==(x, y);
    }
#endif

    [[nodiscard]] constexpr const state_type& state() const& noexcept { return *this; }
    [[nodiscard]]
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    state_type& state() & noexcept { return *this; }

private:
    template<size_t Amount>
    static constexpr result_type rshift(result_type value) noexcept {
#if __cpp_if_constexpr >= 201606L && __cpp_constexpr >= 201304L
        if constexpr (Amount >= word_size) {
            return min();
        } else {
            return value >> Amount;
        }
#else
        return (Amount >= word_size) ? min() : value >> Amount;
#endif
    }

    // Note: May have extra bits over word_size
    template<size_t Amount>
    static constexpr result_type lshift(result_type value) noexcept {
#if __cpp_if_constexpr >= 201606L
        if constexpr (Amount >= word_size) {
            return min();
        } else {
            return value << Amount;
        }
#else
        return (Amount >= word_size) ? min() : value << Amount;
#endif
    }

#if __cpp_constexpr >= 201304L
    constexpr
#endif
    void transition_algorithm() noexcept {
        constexpr result_type upper_bits_mask{ lshift<mask_bits>(max()) & max() };
        constexpr result_type lower_bits_mask{ max() ^ upper_bits_mask };

        for (size_t i = 0; i < state_size; ++i) {
            result_type Y = (X[i] & upper_bits_mask) | (X[static_cast<size_t>(i+1u) % state_size] & lower_bits_mask);
            result_type alpha = xor_mask * (Y bitand 1u);
            X[i] = X[(i + shift_size) % state_size] xor (Y >> 1) xor alpha;
        }

        /*
        for (size_t i = 0; i < (state_size - shift_size); ++i) {
            result_type Y = (X[i] & upper_bits_mask) | (X[i+1u] & lower_bits_mask);
            result_type alpha = xor_mask * (Y bitand 1u);
            X[i] = X[i + shift_size] xor (Y >> 1) xor alpha;
        }

        for (size_t i = (state_size - shift_size); i < state_size - 1u; ++i) {
            result_type Y = (X[i] & upper_bits_mask) | (X[i+1u] & lower_bits_mask);
            result_type alpha = xor_mask * (Y bitand 1u);
            X[i] = X[i + (shift_size - state_size)] xor (Y >> 1) xor alpha;
        }

        result_type Y = (X[state_size - 1u] & upper_bits_mask) | (X[0] & lower_bits_mask);
        result_type alpha = xor_mask * (Y bitand 1u);
        X[state_size - 1u] = X[shift_size - 1u] xor (Y >> 1) xor alpha;
         */
    }
public:
    static
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    result_type scramble(result_type x) noexcept {
        result_type z = x;
        z = z xor (rshift<tempering_u>(z) bitand tempering_d);
        z = z xor (lshift<tempering_s>(z) bitand tempering_b);
        z = z xor (lshift<tempering_t>(z) bitand tempering_c);
        z = z & max();  // (Put z back in range if the lshift brought extra bits)
        z = z xor rshift<tempering_l>(z);

        return z;
    }

#if __cpp_constexpr >= 201304L
    constexpr
#endif
    result_type operator()() noexcept {
        result_type z = scramble(X[i]);
        // transition_algorithm is O(state_size), but it is run every state_size calls to operator()
        if (++i == state_size) [[unlikely]] {
            transition_algorithm();
            i = 0;
        }
        return z;
    }
private:
    template<class InputIt, class Sentinel>
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    void generate_multiple(InputIt& first, Sentinel& last) noexcept(
        noexcept(static_cast<bool>(first == last), static_cast<void>(*first = result_type{}), static_cast<void>(++first))
    ) {
        while (true) {
            if (first == last) return;

            for (; i < state_size-1u;) {
                *first = scramble(X[i++]);
                ++first;
                if (first == last) return;
            }

            result_type z = scramble(X[state_size-1u]);
            transition_algorithm();
            i = 0;
            *first = z;
            ++first;
        }
    }

public:
    /*
     * e(first, last) is exactly equivalent to:
     *   {
     *     auto first_copy = first;
     *     for (; !static_cast<bool>(first_copy == last); ++first_copy) *first_copy = e();
     *     return first_copy;
     *   }
     * But more efficient in terms of code generation (less unnecessary checks)
     */
    template<class InputIt, class Sentinel = InputIt>
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    InputIt operator()(InputIt first, Sentinel last) noexcept(noexcept(generate_multiple(first, last))) {
        generate_multiple(first, last);
        // (Assume iterators are nothrow copyable/movable)
        return first;
    }

    [[nodiscard]]
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    result_type peek(unsigned long long distance = 0) const noexcept {
        if (distance != 0 && (distance >= n - i)) {
            mersenne_twister_engine copy = *this;
            copy.discard(distance-1u);
            return scramble(copy.X[copy.i]);
        }
        return scramble(X[i+distance]);
    }

    template<class InputIt, class Sentinel = InputIt>
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    InputIt peek(InputIt first, Sentinel last, unsigned long distance = 0) const noexcept(noexcept(generate_multiple(first, last))) {
        // Ensure that the sequence written isn't tainted if the iterated object's functions
        // (like operator==, operator* or operator= on the iterated objects) mutate *this
        mersenne_twister_engine copy = *this;
        copy.discard(distance);
        if (first == last) return first;
        for (size_t i = copy->i; i < state_size; ++i) {
            *first = scramble(X[i]);
            ++first;
            if (first == last) return first;
        }

        while (true) {
            copy.transition_algorithm();
            for (result_type x : copy.X) {
                *first = scramble(x);
                ++first;
                if (first == last) return first;
            }
        }
    }

#if __cpp_constexpr >= 201304L
    constexpr
#endif
    void discard(unsigned long long z) noexcept {
        while (z >= state_size) {
            transition_algorithm();
            z -= state_size;
        }
        // Note: Not written as `z >= state_size - i` because optimizer
        // doesn't know that i < state_size so there is code gen for `discard(0)`
        if (z > (state_size - 1u) - i) {
            transition_algorithm();
            i -= state_size;
        }
        i += static_cast<size_t>(z);
    }

    template<class OStream>
    friend OStream& operator<<(OStream& os, const mersenne_twister_engine& x) {
        return os << static_cast<const state_type&>(x);
    }

    template<class IStream>
    friend IStream& operator>>(IStream& is, mersenne_twister_engine& x) {
        IStream& result = is >> static_cast<state_type&>(x);
        if (x.i == state_size) {
            // libstdc++ sets i to state_size instead of immediately transitioning and setting i to 0
            x.transition_algorithm();
            x.i = 0;
        }
        return result;
    }

#ifndef FREESTANDING_MERSENNE_TWISTER_NO_SEED_SEQUENCE

    template<class Sseq, detail::enable_if_seed_sequence<Sseq>* = nullptr>
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    explicit mersenne_twister_engine(Sseq& q) noexcept(noexcept(seed(q))) : state_type{} { seed(q); }

    template<class Sseq>
#if __cpp_constexpr >= 201304L
    constexpr
#endif
    detail::enable_if_seed_sequence<Sseq> seed(Sseq& seq) noexcept(noexcept(seq.generate(static_cast<detail::uint_least32_t*>(nullptr), static_cast<detail::uint_least32_t*>(nullptr)))) {
        constexpr size_t k = w % 32u == 0u ? w / 32u : (w / 32u) + 1u;
        // Limit to these specific integer types since there is
        // no way to check for extended integer types
        if (k == 1u && (result_type_bits >= 32u) && (
                detail::is_same<UIntType, unsigned char>::value ||
                detail::is_same<UIntType, unsigned short>::value ||
                detail::is_same<UIntType, unsigned>::value ||
                detail::is_same<UIntType, unsigned long>::value ||
                detail::is_same<UIntType, unsigned long long>::value)) {
            // Directly use seed sequence
            seq.generate(X + 0, X + n*k);
        } else {
            detail::uint_least32_t A[n * k]
#if __cpp_constexpr >= 201907L
                // No initialisation needed
#else
                {}
#endif
            ;
            seq.generate(A + 0, A + n*k);
            const detail::uint_least32_t* p = A;

            for (result_type& x : X) {
                x = min();
                for (size_t j = 0; j < k; ++j) {
                    x |= static_cast<result_type>(*p++) << (32u * j);
                }
                x &= max();
            }
        }

        if (rshift<r>(X[0]) == min()) {
            bool all_zero = true;
            for (size_t i = 1; i < n; ++i) {
                if (X[i] != min()) {
                    all_zero = false;
                    break;
                }
            }

            if (all_zero) {
                X[0] = max();
            }
        }

        transition_algorithm();
        i = 0;
    }

#endif

};

#if __cpp_inline_variables < 201606L
template<class UIntType, size_t n>
constexpr size_t mersenne_twister_state<UIntType, n>::state_size;

template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
constexpr size_t mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>::word_size;
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
constexpr size_t mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>::shift_size;
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
constexpr size_t mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>::mask_bits;
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
constexpr UIntType mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>::xor_mask;
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
constexpr size_t mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>::tempering_u;
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
constexpr UIntType mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>::tempering_d;
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
constexpr size_t mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>::tempering_s;
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
constexpr UIntType mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>::tempering_b;
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
constexpr size_t mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>::tempering_t;
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
constexpr UIntType mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>::tempering_c;
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
constexpr size_t mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>::tempering_l;
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
constexpr UIntType mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>::initialization_multiplier;
template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
constexpr UIntType mersenne_twister_engine<UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f>::default_seed;
#endif

template<class UInt32 = detail::uint_least32_t>
using mt19937 =
        mersenne_twister_engine<UInt32, 32, 624, 397, 31,
                0x9908'b0dfu, 11, 0xffff'ffffu, 7, 0x9d2c'5680u, 15, 0xefc6'0000u, 18, 1'812'433'253u>;

template<class UInt64 = detail::uint_least64_t>
using mt19937_64 =
        mersenne_twister_engine<UInt64, 64, 312, 156, 31,
                0xb502'6f5a'a966'19e9u, 29, 0x5555'5555'5555'5555u, 17,
                0x71d6'7fff'eda6'0000u, 37, 0xfff7'eee0'0000'0000u, 43, 6'364'136'223'846'793'005u>;

#ifdef FREESTANDING_MERSENNE_TWISTER_SELF_TEST
#if __cpp_constexpr >= 201304L
static_assert(mt19937<>{}.peek(10000) == 4123659995u, "Did not pass self test");
static_assert(mt19937_64<>{}.peek(10000) == 9981545732273789042u, "Did not pass self test");
#else
#warning FREESTANDING_MERSENNE_TWISTER_SELF_TEST: Need C++14 and modern compiler to perform self test
#endif
#endif

}

#endif
