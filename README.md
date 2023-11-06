# freestanding_mersenne_twister

Implements the C++11 standard library `mersenne_twister_engine` with
absolutely no dependence on any other header files.

Also has some extended functionality.

## Quick start

```c++
// Use exactly like std::mt19937 (It generates the same numbers)
using mt19937 = freestanding_mersenne_twister::mt19937<>;

// Likewise for std::mt19937_64
using mt19937_64 = freestanding_mersenne_twister::mt19937_64<>;
```

## Compatibility notes

 * `operator>>`/`operator<<` are different, see below. But they should not be used and
   replaced with manipulating `state()`. These were implementation defined to begin with,
   but you *should* still be able to use it with libstdc++'s `std::mersene_twister_engine<...>`.
 * All functions (except `operator>>`/`operator<<`) are now `constexpr` in C++14 and up.
 * `mt19937<>` is a template alias that uses `unsigned` by default. The standard
   library version would be `mt19937<std::uint_fast32_t>`. The standard version is
   grossly inefficient if `uint_fast32_t` is 64 bit since the state array would double in size.
 * Similar for `mt19937_64<>`, with `unsigned long long` instead of `std::uint_fast64_t`.

## API

### `mersenne_twister_engine`

```c++
namespace freestanding_mersenne_twister {

template<class UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f>
struct mersenne_twister_engine;

template<class UInt32 = unsigned>
using mt19937 =
        mersenne_twister_engine<UInt32, 32, 624, 397, 31,
                0x9908'b0dfu, 11, 0xffff'ffffu, 7, 0x9d2c'5680u, 15, 0xefc6'0000u, 18, 1'812'433'253u>;

template<class UInt64 = unsigned long long>
using mt19937_64 =
        mersenne_twister_engine<UInt64, 64, 312, 156, 31,
                0xb502'6f5a'a966'19e9u, 29, 0x5555'5555'5555'5555u, 17,
                0x71d6'7fff'eda6'0000u, 37, 0xfff7'eee0'0000'0000u, 43, 6'364'136'223'846'793'005u>;

}
```

`mt19937<T>`, where `T` is an integral type that has a bit width of 32 or greater,
is an engine which produces the same sequence as `std::mt19937`.

`mt19937_64<T>` is the same with an at-least 64 bit integral type and `std::mt19937_64`.

No exceptions are thrown by the engine itself but operations on arguments passed in
might throw (for example, inserting into a stream or incrementing an iterator).

(Constructor that takes a seed sequence and seed function that do the same only exist
if the macro `FREESTANDING_MERSENNE_TWISTER_NO_SEED_SEQUENCE` is not defined before
including the header the first time. These are not documented and work as you expect)

#### Member types

 * `result_type`: The template parameter `UIntType`. This is the type of the value
   generated by the engine. It must be an unsigned integral type.
 * `state_type`: `mersenne_twister_engine` inherits from this types. It is an aggregate
   type `mersnne_twister_state<result_type, state_size>` that stores the
   current state of the engine (see below).

#### Functions

```c++
// Constructors
constexpr mersenne_twister_engine() noexcept : mersenne_twister_engine(default_seed) {}
constexpr explicit mersenne_twister_engine(result_type value) noexcept;
/* implicitly-declared */ constexpr mersenne_twister_engine(const mersenne_twister_engine&) noexcept = default;
constexpr explicit mersenne_twister_engine(const state_type& state) noexcept;
```

The integer used to construct a `mersenne_twister_engine` (defaulting to `default_seed`)
seeds the random sequence. Any two `mersenne_twister_engine`s constructed with the same
seed are equal (i.e., will produce the same sequence of numbers).

The implicitly-declared copy constructor is trivial and will produce a `mersenne_twister_engine`
equal to its argument.

If constructed from a state, the underlying state will be equal to the passed in state.
Equivalent to `this->state() = state;`.

```c++
// Copy-assign
/* implicitly-declared */ constexpr mersenne_twister_engine& operator=(const mersenne_twister_engine&) noexcept = default;
```

`mersenne_twister_engine` objects are trivially copyable: This copies each element of the state,
including the index `i`. `*this` will subsequently be equal to the argument passed in.

```c++
constexpr void seed(result_type value = default_seed) noexcept;
```

Equivalent to `*this = mersenne_twister(value);`. The sequence will be from the beginning.

```c++
constexpr result_type operator()() noexcept;
template<class InputIt, class Sentinel = InputIt>
constexpr InputIt operator()(InputIt first, Sentinel last) noexcept(/*if possible*/);
```

The first overload advances the state of the engine and returns the next value.  
The second overload writes successive values while advancing the state, similar to
`std::ranges::generate(first, last, *this)`, but more efficiently.

This takes (amortised) `O(1)` time for each number generated (so the second overload
is `O(std::ranges::distance(first, last))`).

```c++
constexpr result_type peek(unsigned long long distance = 0) const noexcept;
template<class InputIt, class Sentinel = InputIt>
constexpr InputIt peek(InputIt first, Sentinel last, unsigned long distance = 0) const noexcept (/*if possible*/);
```

The first overload will return what calling `operator()` would do after calling
`operator()` `distance` times without actually changing the state of `*this`.
The second overload does the same except it writes to the range [first, last) what
successive invocations of `operator()` would return.

`peek(0)` takes `O(1)` time. For any other value, it takes `O(distance + state_size)` time.  
The second overload takes `O(distance + std::ranges::distance(first, last) + state_size)` time.

```c++
constexpr void discard(unsigned long long z) noexcept;
```

Equivalent to calling `operator()` `z` times and discarding the results: Efficiently
advances the state of the generator `z` times.

```c++
friend constexpr bool operator==(const mersenne_twister_engine&, const mersenne_twister&) noexcept;
```

In C++23, this is trivial and just compares each member of the state and the index.  
Before that, this emualates that (comparing each object in the array in the state and
then the index), and an `operator!=` is provided too that returns the negation of this.

When two `mersenne_twister_engine` objects are equal, they will generate the same
sequence of random numbers.

```c++
template<class OStream>
friend OStream& operator<<(OStream& os, const mersenne_twister_engine& x);
template<class IStream>
friend IStream& operator>>(IStream& is, mersenne_twister_engine& x);
```

This is one function that diverges from `std::mersenne_twister_engine`. Since the
definition of `std::basic_ostream`/`std::basic_istream` isn't available, a generic
placeholder is used. When actually called, these should be a `basic_ostream`/`basic_istream`.

Will output each element of the state array seperated by a space, followed by the current index.  
Will input something in the same format.

Reading the characters previously written to a stream will produce an equal
`mersenne_twister_engine`. However, it is more efficient to directly serialize
the bytes of the state of the engine (taking care with byte order if it is
going to be read on a different system).

```c++
constexpr const state_type& state() const &;
constexpr state_type& state() &;
```

Returns the aggregate type holding the entire state of `*this`.

```c++
static constexpr result_type scramble(result_type x) noexcept;
```

Scrambles the argument according to the parameters given in the template arguments.
This is the function called on all members of the state before returning them
(i.e., `e.peek() == decltype(e)::scramble(e.X[e.i])`).

```c++
static constexpr result_type min() noexcept { return 0; }
static constexpr result_type max() noexcept {
    // (numeric_limits not actually used, this is for illustration purposes)
    using limits = std::numeric_limits<result_type>;
    return word_size == limits::digits ? limits::max() : (result_type(1) << word_size) - result_type(1);
}
```

The range of the produced values (inclusive). `max()` returns the value of 2<sup>`word_size`</sup>-1,
guarding against overflow in the power.

#### Non-static data members

 * `result_type X[state_size];`: `state_size` `word_size` bit integers, used
   to produce each random number as well as the next state.
 * `size_t i;`: An index where `i < state_size` which decides which of the elements
   of `X` is going to produce the next number. Increments after calls to `operator()`
   and others, set to `0` after it reaches `state_size`.

#### Static data members

 * `static constexpr size_t word_size;`: The template parameter `w`.
 * `static constexpr size_t state_size;`: The template parameter `n`.
 * `static constexpr size_t shift_size;`: The template parameter `m`.
 * `static constexpr size_t mask_bits;`: The template parameter `r`.
 * `static constexpr result_type xor_mask;`: The template parameter `a`.
 * `static constexpr size_t tempering_u;`: The template parameter `u`.
 * `static constexpr result_type tempering_d;`: The template parameter `d`.
 * `static constexpr size_t tempering_s;`: The template parameter `s`.
 * `static constexpr result_type tempering_b;`: The template parameter `b`.
 * `static constexpr size_t tempering_t;`: The template parameter `t`.
 * `static constexpr result_type tempering_c;`: The template parameter `c`.
 * `static constexpr size_t tempering_l;`: The template parameter `l`.
 * `static constexpr result_type initialization_multiplier;`: The template parameter `f`.
 * `static constexpr result_type default_seed;`: A constant, `5489u`.

### `mersenne_twister_state`

```c++
namespace freestanding_mersenne_twister {

}
// A trivial aggregate type
template<class UIntType, size_t n>
struct mersenne_twister_state {
    using result_type = UIntType;

    static constexpr size_t state_size = n;

    result_type X[state_size];
    size_t i;
};

}
```

`X` and `i` are described above. `mersenne_twister_engine<UIntType, n, ...>` inherits from
`mersenne_twister_state<UIntType, n>` to get its data members `X` and `i`.

No protection is provided if these are set incorrectly. It is undefined behaviour if
a member function of `mersenne_twister_engine` is called and `i >= state_size`.

If a elements of `X` is out of range of `word_size`, the numbers generated may also be
greater than `word_size`, but the behaviour is otherwise not undefined.

There are also non-member functions `operator==` and `operator<<`/`operator>>`
that work the same as there corresponding versions in `mersenne_twister_engine`.

## Example

```c++
using mt19937 = freestanding_mersenne_twister::mt19937<unsigned long>;
using mt19937_64 = freestanding_mersenne_twister::mt19937_64<>;
static_assert(mt19937{}.peek(10000) == 4123659995u);
static_assert(mt19937_64{}.peek(10000) == 9981545732273789042u);

int main() {
    mt19937_64 e(1234);

    (void) e();
    auto x = e();
    auto y = e.peek();
    auto z = e();
    assert(y == z);

    mt19937_64 e2;
    e2.seed(1234);
    e2.discard(1);
    unsigned long xy[2];
    e2(xy, xy + 2);

    assert(xy[0] == x && xy[1] == y);

    auto e3 = e2;
    assert(e3 == e2 && e3() == e2());

    write_to_file("path/to/file.bin", reinterpret_cast<const char*>(&e3.state()), sizeof(e3.state()));
    mt19937_64 e4;
    read_from_file("path/to/file.bin", reinterpret_cast<char*>(&e4.state()), sizeof(e4.state()));
    assert(e3 == e4 && e3() == e4());
}
```
